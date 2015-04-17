#include "videoreader.hpp"

#define WINDOW_NAME "video"

VideoReader::VideoReader(std::string filename) {
  videoCapture.open(filename);
  if (!videoCapture.isOpened()) {
    std::cerr << "Failed to open file " << filename << std::endl;
    std::exit(1);
  }
  framesCaptured = 0;
  windowCreated = false;
}

cv::Mat VideoReader::getFrame() {
  cv::Mat frame;
  videoCapture >> frame;
  if (framesCaptured == 0 && frame.empty()) {
    videoCapture >> frame;
  }
  if (!frame.empty()) {
    framesCaptured++;
  }
  return frame;
}

bool VideoReader::showFrame() {
  if (!windowCreated) {
    cv::namedWindow(WINDOW_NAME, CV_WINDOW_AUTOSIZE);
    windowCreated = true;
  }

  double start = Timer::time();

  cv::Mat frame = getFrame();
  if (frame.empty()) {
    std::cout << "No frames left to show." << std::endl;
    return false;
  }

  double end = Timer::time();
  double fps = 1.0 / ((end - start) / 1000.0);
  avgFps = ((avgFps * (double) framesCaptured) + fps) /
    ((double) framesCaptured + 1.0);
  std::cout << avgFps << " Average FPS" << std::endl;

  cv::imshow(WINDOW_NAME, frame);
  return true;
}
