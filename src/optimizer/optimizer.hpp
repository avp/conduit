#ifndef OPTIMIZER_OPTIMIZER_H_
#define OPTIMIZER_OPTIMIZER_H_

#include <opencv2/highgui/highgui.hpp>

class OptimizedImage {
  friend class Optimizer;

  public:
    size_t size();

  private:
    OptimizedImage(const cv::Mat& image);
    cv::Mat image;
};

class Optimizer {
  public:
    static OptimizedImage optimizeImage(const cv::Mat& image);
    static cv::Mat extractImage(const OptimizedImage& image);
};

#endif
