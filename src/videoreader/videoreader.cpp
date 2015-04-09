#include "videoreader.hpp"

#define WINDOW_NAME "video"

VideoReader::VideoReader(std::string filename) {
  videoCapture.open(filename);
  if (!videoCapture.isOpened()) {
    std::cerr << "Failed to open file " << filename << std::endl;
    std::exit(1);
  }
  cv::namedWindow(WINDOW_NAME, CV_WINDOW_AUTOSIZE);
}

cv::Mat VideoReader::getFrame() {
  cv::Mat frame;
  videoCapture >> frame;
  return frame;
}

bool VideoReader::showFrame() {
  cv::Mat frame = getFrame();
  if (frame.empty()) {
    std::cout << "No frames left to show." << std::endl;
    return false;
  }
  cv::imshow(WINDOW_NAME, frame);
  return true;
}
