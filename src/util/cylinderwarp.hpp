#ifndef UTIL_CYLINDERWARP_H_
#define UTIL_CYLINDERWARP_H_

#include <opencv2/highgui/highgui.hpp>

class CylinderWarp {
  public:
    static cv::Mat cylinderWarp(const cv::Mat& image);

  private:
    static cv::Point2f warpPoint(cv::Point2f point, int width, int height);
};

#endif
