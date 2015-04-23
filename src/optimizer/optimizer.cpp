#include "optimizer.hpp"

using cv::Mat;

OptimizedImage::OptimizedImage(const Mat& image) {
  this->image = Mat(image);
}

size_t OptimizedImage::size() {
  return image.step[0] * image.rows;
}

OptimizedImage Optimizer::optimizeImage(const Mat& image) {
  OptimizedImage optImage(image);
  return optImage;
}

Mat Optimizer::extractImage(const OptimizedImage& optImage) {
  return Mat(optImage.image);
}
