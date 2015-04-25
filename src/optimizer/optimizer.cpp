#include "optimizer.hpp"

#include <iostream>

using cv::Mat;

static const int CROP_ANGLE = 120;

OptimizedImage::OptimizedImage(const Mat& image) {
  this->image = Mat(image);
}

size_t OptimizedImage::size() {
  // return image.step[0] * image.rows;
  return image.total() * image.elemSize();
}

OptimizedImage Optimizer::optimizeImage(const Mat& image, int angle) {
  angle = std::min(angle, 360 - (CROP_ANGLE / 2));
  angle = std::max(angle, CROP_ANGLE / 2);

  int width = image.cols;

  int leftAngle = angle - CROP_ANGLE / 2;
  int rightAngle = angle + CROP_ANGLE / 2;

  int leftCol = leftAngle / 360.0 * width;
  int rightCol = rightAngle / 360.0 * width;

  std::cout << leftCol << " <-> " << rightCol << std::endl;

  Mat cropped = Mat(image, cv::Range::all(), cv::Range(leftCol, rightCol));

  OptimizedImage optImage(cropped);
  return optImage;
}

Mat Optimizer::extractImage(const OptimizedImage& optImage) {
  return Mat(optImage.image);
}
