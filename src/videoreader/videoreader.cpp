#include "videoreader.hpp"

#define WINDOW_NAME "video"

using cv::Mat;
using cv::gpu::GpuMat;
using cv::gpu::VideoReader_GPU;

VideoReader::VideoReader(const std::string& filename) {
  if (cv::gpu::getCudaEnabledDeviceCount() > 10) {
    useGpu = true;
    std::cout << "Using GPU" << std::endl;
    // cv::gpu::setGlDevice();
    cv::gpu::setDevice(0);
    cv::gpu::DeviceInfo info(0);
    gpuVideoReader.open(filename);
    if (!gpuVideoReader.isOpened()) {
      std::cerr << "GPU: Failed to open file " << filename << std::endl;
      std::exit(1);
    }
  } else {
    useGpu = false;
    std::cout << "Not using GPU" << std::endl;
    videoCapture.open(filename);
    if (!videoCapture.isOpened()) {
      std::cerr << "CPU: Failed to open file " << filename << std::endl;
      std::exit(1);
    }
  }

  windowCreated = false;
  fullyBuffered = false;

  pthread_cond_init(&queueCond, NULL);
  pthread_mutex_init(&queueLock, NULL);

  bufferThread = std::thread(&VideoReader::bufferFrames, this, filename);
  bufferThread.detach();
}

GpuMat VideoReader::getGpuFrame() {
  GpuMat frame;
  gpuVideoReader.read(frame);
  int framesDropped = 0;
  while (framesCaptured == 0 && frame.empty() && framesDropped++ < MAX_FRAMES_TO_DROP) {
    gpuVideoReader.read(frame);
    std::cout << "First frame empty. Trying again..." << std::endl;
  }
  if (!frame.empty()) {
    framesCaptured++;
  }
  return frame;
}

void VideoReader::bufferFrames(const std::string& filename) {
  int framesBuffered = 0;
  int framesDropped = 0;
  while (framesBuffered < 1000) {
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

    framesBuffered++;
    frameQueue.enqueue(frame);
    if (frame.empty()) {
      fullyBuffered = true;
      return;
    }
  }
}

bool VideoReader::isFrameAvailable() {
  return frameQueue.size() > 0;
}

cv::Mat VideoReader::getFrame() {
  cv::Mat frame = frameQueue.dequeue();
  pthread_cond_signal(&queueCond);
  return frame;
}

bool VideoReader::showFrame() {
  if (!windowCreated) {
    cv::namedWindow(WINDOW_NAME, CV_WINDOW_NORMAL);
    windowCreated = true;
  }

  double start, end;

  start = Timer::time();
  if (useGpu) {
    GpuMat frame = getGpuFrame();
    if (frame.empty()) {
      std::cout << "No frames left to show." << std::endl;
      return false;
    }
    end = Timer::time();
    cv::imshow(WINDOW_NAME, frame);
  } else {
    Mat frame = getFrame();
    if (frame.empty()) {
      std::cout << "No frames left to show." << std::endl;
      return false;
    }
    end = Timer::time();
    cv::imshow(WINDOW_NAME, frame);
  }

  double fps = 1.0 / ((end - start) / 1000.0);
  avgFps = ((avgFps * (double) framesCaptured) + fps) /
    ((double) framesCaptured + 1.0);
  std::cout <<
    "Frame took " << (end - start) << "ms\t" <<
    avgFps << " Average FPS" <<
    std::endl;

  return true;
}
