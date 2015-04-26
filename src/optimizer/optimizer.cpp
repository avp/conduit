#include "optimizer.hpp"

#include <iostream>

using cv::Mat;
using cv::Range;
using cv::Size;

static const int CROP_ANGLE = 120;
static const int FOCUS_WIDTH = 800;
static const int BLUR_FACTOR = 5;

OptimizedImage::OptimizedImage(const Mat& focused,
    const Mat& blurredLeft,
    const Mat& blurredRight,
    Size origSize) {
  this->focused = Mat(focused);
  this->blurredLeft = Mat(blurredLeft);
  this->blurredRight = Mat(blurredRight);
  this->origSize = origSize;
}

size_t OptimizedImage::size() {
  return ImageUtil::imageSize(focused) +
    ImageUtil::imageSize(blurredLeft) +
    ImageUtil::imageSize(blurredRight);
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

  leftCol = cropped.cols / 2 - FOCUS_WIDTH / 2;
  rightCol = cropped.cols / 2 + FOCUS_WIDTH / 2;

  Mat focused = Mat(cropped, Range::all(), Range(leftCol, rightCol));
  Mat left = Mat(cropped, Range::all(), Range(0, leftCol));
  Mat right = Mat(cropped, Range::all(), Range(rightCol, cropped.cols));

  Size origSize(left.size());

  Mat blurredLeft = left;
  Mat blurredRight = right;

  Size smallSize(left.cols / BLUR_FACTOR, left.rows / BLUR_FACTOR);
  cv::resize(left, blurredLeft, smallSize);
  cv::resize(right, blurredRight, smallSize);

  OptimizedImage optImage(focused, blurredLeft, blurredRight, origSize);
  return optImage;
}

Mat Optimizer::extractImage(const OptimizedImage& optImage) {
  Mat tmp, fullImage;

  Mat left, right;

  cv::resize(optImage.blurredLeft, left, optImage.origSize);
  cv::resize(optImage.blurredRight, right, optImage.origSize);

  hconcat(left, optImage.focused, tmp);
  hconcat(tmp, right, fullImage);
  return fullImage;
}
