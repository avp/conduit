#include "optimizer.hpp"

#include <iostream>

using cv::Mat;
using cv::Range;
using cv::Size;

static const int CROP_ANGLE = 120;
static const int FOCUS_ANGLE = 20;
static const int BLUR_FACTOR = 5;

OptimizedImage::OptimizedImage(const Mat& focused,
    const Mat& blurredLeft,
    const Mat& blurredRight,
    Size origSize,
    Size fullSize,
    int origType) {
  this->focused = Mat(focused);
  this->blurredLeft = Mat(blurredLeft);
  this->blurredRight = Mat(blurredRight);
  this->origSize = origSize;
  this->fullSize = fullSize;
  this->origType = origType;
}

size_t OptimizedImage::size() {
  return ImageUtil::imageSize(focused) +
    ImageUtil::imageSize(blurredLeft) +
    ImageUtil::imageSize(blurredRight);
}

OptimizedImage Optimizer::optimizeImage(const Mat& image, int angle) {
  REQUIRES(0 <= angle && angle < 360);
  REQUIRES(FOCUS_ANGLE < CROP_ANGLE);

  const int width = image.cols;
  const double angleToWidth = width / 360.0;

  int leftAngle = (angle - CROP_ANGLE / 2) % 360;
  int rightAngle = (angle + CROP_ANGLE / 2) % 360;

  while (leftAngle < 0) {
    leftAngle += 360;
  }
  ASSERT(0 <= leftAngle && leftAngle < 360);
  ASSERT(0 <= rightAngle && rightAngle < 360);

  int leftCol = leftAngle * angleToWidth;
  int rightCol = rightAngle * angleToWidth;

  Mat cropped;
  if (leftCol < rightCol) {
    cropped = Mat(image, Range::all(), Range(leftCol, rightCol));
  } else {
    Mat leftMat = Mat(image, Range::all(), Range(leftCol, width));
    Mat rightMat = Mat(image, Range::all(), Range(0, rightCol));
    hconcat(leftMat, rightMat, cropped);
  }

  int focusWidth = FOCUS_ANGLE * angleToWidth;

  leftCol = cropped.cols / 2 - focusWidth / 2;
  rightCol = cropped.cols / 2 + focusWidth / 2;

  ASSERT(0 <= leftCol);
  ASSERT(leftCol <= rightCol);
  ASSERT(rightCol < cropped.cols);
  Mat focused = Mat(cropped, Range::all(), Range(leftCol, rightCol));
  Mat left = Mat(cropped, Range::all(), Range(0, leftCol));
  Mat right = Mat(cropped, Range::all(), Range(rightCol, cropped.cols));

  Size origSize(left.size());

  Mat blurredLeft = left;
  Mat blurredRight = right;

  Size smallSize(left.cols / BLUR_FACTOR, left.rows / BLUR_FACTOR);
  cv::resize(left, blurredLeft, smallSize);
  cv::resize(right, blurredRight, smallSize);

  OptimizedImage optImage(focused, blurredLeft, blurredRight, origSize, image.size(), image.type());
  return optImage;
}

Mat Optimizer::extractImage(const OptimizedImage& optImage) {
  Mat tmp, croppedImage;

  Mat left, right;

  cv::resize(optImage.blurredLeft, left, optImage.origSize);
  cv::resize(optImage.blurredRight, right, optImage.origSize);

  hconcat(left, optImage.focused, tmp);
  hconcat(tmp, right, croppedImage);

  Mat tmp2, fullImage;

  int leftBufferRows = croppedImage.size().height;
  int leftBufferCols = (optImage.fullSize.width - croppedImage.size().width)/2;
  Mat leftBuffer(leftBufferRows, leftBufferCols, optImage.origType);
  hconcat(leftBuffer, croppedImage, tmp2);
  int rightBufferCols = optImage.fullSize.width - tmp2.size().width;
  Mat rightBuffer(leftBufferRows, rightBufferCols, optImage.origType);
  hconcat(tmp2, rightBuffer, fullImage);

  return fullImage;
}
