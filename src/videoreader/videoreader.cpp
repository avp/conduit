#include "videoreader.hpp"

#include "../optimizer/optimizer.hpp"

#define WINDOW_NAME "video"

VideoReader::VideoReader(const std::string& filename) {
  videoCapture.open(filename);
  if (!videoCapture.isOpened()) {
    std::cerr << "Failed to open file " << filename << std::endl;
    std::exit(1);
  }

  windowCreated = false;
  fullyBuffered = false;

  pthread_cond_init(&queueCond, NULL);
  pthread_mutex_init(&queueLock, NULL);

  bufferThread = std::thread(&VideoReader::bufferFrames, this, filename);
  bufferThread.detach();
}

void VideoReader::bufferFrames(const std::string& filename) {
  int framesBuffered = 0;
  int framesDropped = 0;
  while (true) {
    if (frameQueue.size() > 10) {
      pthread_mutex_lock(&queueLock);
      pthread_cond_wait(&queueCond, &queueLock);
      pthread_mutex_unlock(&queueLock);
    }
    cv::Mat frame;
    videoCapture >> frame;
    bool hadError = false;
    while (framesBuffered == 0 && frame.empty() && framesDropped++ < MAX_FRAMES_TO_DROP) {
      videoCapture >> frame;
      std::cout << "First frame empty. Trying again..." << std::endl;
      hadError = true;
    }
    if (hadError && !frame.empty()) {
      std::cout << "First frame retrieved successfully." << std::endl;
    }

    // important to check isDone before changing frame, or it'll be different!
    if (frame.empty()) {
      fullyBuffered = true;
      return;
    }
    framesBuffered++;

    // if (autoOptimize)
    //   frame = Optimizer::processImage(frame, optimizeAngle, 90);
    frameQueue.enqueue(frame);
  }
}

bool VideoReader::isFrameAvailable() {
  return frameQueue.size() > 0;
}

cv::Mat VideoReader::getFrame() {
#ifdef ASYNC_VIDEOCAPTURE
  cv::Mat frame = frameQueue.dequeue();
  pthread_cond_signal(&queueCond);
#else
  cv::Mat frame;
  videoCapture >> frame;
  bool hadError = false;
  int framesDropped = 0;
  while (frame.empty() && framesDropped++ < MAX_FRAMES_TO_DROP) {
    videoCapture >> frame;
    std::cout << "First frame empty. Trying again..." << std::endl;
    hadError = true;
  }
  if (hadError && !frame.empty()) {
    std::cout << "First frame retrieved successfully." << std::endl;
  }
#endif
  return frame;
}



static FramerateProfiler videoReaderProfiler;

bool VideoReader::showFrame() {
  if (!windowCreated) {
    cv::namedWindow(WINDOW_NAME, CV_WINDOW_NORMAL);
    windowCreated = true;
  }

  double start = Timer::time();
  cv::Mat frame = getFrame();
  double end = Timer::time();

  if (frame.empty()) {
    std::cout << "No frames left to show." << std::endl;
    return false;
  }

  double fps = 1.0 / ((end - start) / 1000.0);
  avgFps = ((avgFps * (double) framesCaptured) + fps) /
    ((double) framesCaptured + 1.0);
  std::cout << "Frame took " << (end - start) << "ms; " << avgFps << " Average FPS" << std::endl;

  cv::imshow(WINDOW_NAME, frame);
  return true;
}
