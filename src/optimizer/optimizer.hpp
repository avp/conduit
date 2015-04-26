#ifndef OPTIMIZER_OPTIMIZER_H_
#define OPTIMIZER_OPTIMIZER_H_

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "../util/imageutil.hpp"

class OptimizedImage {
  friend class Optimizer;

  public:
    size_t size();

  private:
    OptimizedImage(const cv::Mat& focused,
        const cv::Mat& blurredLeft,
        const cv::Mat& blurredRight,
        cv::Size origSize);
    cv::Mat image;
    cv::Mat focused;
    cv::Mat blurredLeft;
    cv::Mat blurredRight;
    cv::Size origSize;
};

class Optimizer {
  public:
    static OptimizedImage optimizeImage(const cv::Mat& image, int angle);
    static cv::Mat extractImage(const OptimizedImage& image);
};

#endif
