#ifndef OPTIMIZER_OPTIMIZER_H_
#define OPTIMIZER_OPTIMIZER_H_

#include <vector>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "../util/imageutil.hpp"
#include "../timer/timer.hpp"
#include "../contracts.h"

class OptimizedImage {
  friend class Optimizer;

  public:
    size_t size() const;

  private:
    OptimizedImage(const cv::Mat& focused,
        const cv::Mat& blurredLeft,
        const cv::Mat& blurredRight,
        const cv::Mat& blurredTop,
        const cv::Mat& blurredBottom,
        cv::Size origHSize,
        cv::Size origVSize,
        cv::Size fullSize,
        int leftBuffer);

    cv::Mat focused;
    cv::Mat blurredLeft;
    cv::Mat blurredRight;
    cv::Mat blurredTop;
    cv::Mat blurredBottom;
    cv::Size origHSize;
    cv::Size origVSize;
    cv::Size fullSize;
    int leftBuffer;
};

class Optimizer {
  public:
    static OptimizedImage optimizeImage(const cv::Mat& image,
        int angle, int vAngle);
    static cv::Mat extractImage(const OptimizedImage& image);
};

#endif
