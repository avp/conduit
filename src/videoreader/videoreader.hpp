#ifndef VIDEOREADER_VIDEOREADER_H_
#define VIDEOREADER_VIDEOREADER_H_

#include <iostream>
#include <string>

#include <opencv2/highgui/highgui.hpp>

class VideoReader {
  public:
    VideoReader(std::string filename);
    cv::Mat getFrame();
    bool showFrame();

  private:
    cv::VideoCapture videoCapture;
};

#endif
