#include "optimizer.hpp"

using cv::Mat;
using cv::Range;

static const int CROP_ANGLE = 120;

OptimizedImage::OptimizedImage(const Mat& image) {
  this->image = Mat(image);
}

size_t OptimizedImage::size() {
  return ImageUtil::imageSize(image);
}

OptimizedImage Optimizer::optimizeImage(const Mat& image, int angle) {
  int width = image.cols;

  int leftAngle = (angle - CROP_ANGLE / 2) % 360;
  int rightAngle = (angle + CROP_ANGLE / 2) % 360;

  while (leftAngle < 0) {
    leftAngle += 360;
  }

  int leftCol = leftAngle / 360.0 * width;
  int rightCol = rightAngle / 360.0 * width;

  Mat cropped;
  if (leftCol < rightCol) {
    cropped = Mat(image, Range::all(), Range(leftCol, rightCol));
  } else {
    Mat leftMat = Mat(image, Range::all(), Range(leftCol, width));
    Mat rightMat = Mat(image, Range::all(), Range(0, rightCol));
    hconcat(leftMat, rightMat, cropped);
  }

  OptimizedImage optImage(cropped);
  return optImage;
}

Mat Optimizer::extractImage(const OptimizedImage& optImage) {
  return Mat(optImage.image);
}
