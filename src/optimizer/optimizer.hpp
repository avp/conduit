#ifndef OPTIMIZER_OPTIMIZER_H_
#define OPTIMIZER_OPTIMIZER_H_

#include <vector>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "../util/imageutil.hpp"
#include "../contracts.h"

class OptimizedImage {
  friend class Optimizer;

  public:
    size_t size() const;

  private:
    OptimizedImage(const cv::Mat& focused,
        const cv::Mat& blurredLeft,
        const cv::Mat& blurredRight,
        cv::Size origSize,
        cv::Size fullSize,
        int origType,
        int leftBuffer);
    cv::Mat image;
    cv::Mat focused;
    cv::Mat blurredLeft;
    cv::Mat blurredRight;
    cv::Size origSize;
    cv::Size fullSize;
    int origType;
    int leftBuffer;
};

class Optimizer {
  public:
    static OptimizedImage optimizeImage(const cv::Mat& image, int angle);
    static cv::Mat extractImage(const OptimizedImage& image);
};

#endif
