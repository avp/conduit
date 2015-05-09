#ifndef OPTIMIZER_OPTIMIZER_H_
#define OPTIMIZER_OPTIMIZER_H_

#include <vector>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "../util/imageutil.hpp"
#include "../util/timer.hpp"
#include "../contracts.h"

class OptimizedImage {
  friend class Optimizer;

  public:
    size_t size() const;

  private:
    OptimizedImage(const cv::Mat& focused,
        const cv::Mat& blurred,
        int focusRow, int focusCol,
        cv::Size fullSize, int leftBuffer);

    cv::Mat focused;
    cv::Mat blurred;
    int focusRow;
    int focusCol;
    cv::Size fullSize;
    int leftBuffer;
};

class Optimizer {
  public:
    static OptimizedImage optimizeImage(const cv::Mat& image,
        int angle, int vAngle);
    static cv::Mat extractImage(const OptimizedImage& image);
    static cv::Mat processImage(const cv::Mat& image,
        int angle, int vAngle);
};

#endif
