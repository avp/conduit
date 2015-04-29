#include "optimizer.hpp"

#include <iostream>

using std::vector;

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
    int origType,
    int leftBuffer) {
  this->focused = Mat(focused);
  this->blurredLeft = Mat(blurredLeft);
  this->blurredRight = Mat(blurredRight);
  this->origSize = origSize;
  this->fullSize = fullSize;
  this->origType = origType;
  this->leftBuffer = leftBuffer;
}

size_t OptimizedImage::size() const {
  return ImageUtil::imageSize(focused) +
    ImageUtil::imageSize(blurredLeft) +
    ImageUtil::imageSize(blurredRight);
}

OptimizedImage Optimizer::optimizeImage(const Mat& image, int angle) {
  if (angle < 0)
    angle += 360;
  else if (angle > 360)
    angle -= 360;

  REQUIRES(0 <= angle && angle < 360);
  REQUIRES(FOCUS_ANGLE < CROP_ANGLE);

//  std::cout << angle << std::endl;

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
  ASSERT(0 <= leftCol);
  ASSERT(leftCol < width);
  ASSERT(0 <= rightCol);
  ASSERT(rightCol < width);

  Mat cropped;
  if (leftCol < rightCol) {
    cropped = Mat(image, Range::all(), Range(leftCol, rightCol));
  } else {
    Mat leftMat = Mat(image, Range::all(), Range(leftCol, width));
    Mat rightMat = Mat(image, Range::all(), Range(0, rightCol));
    hconcat(leftMat, rightMat, cropped);
  }

  int focusWidth = FOCUS_ANGLE * angleToWidth;

  int focusLeftCol = cropped.cols / 2 - focusWidth / 2;
  int focusRightCol = cropped.cols / 2 + focusWidth / 2;

  ASSERT(0 <= focusLeftCol);
  ASSERT(focusLeftCol <= focusRightCol);
  ASSERT(focusRightCol < cropped.cols);
  Mat focused = Mat(cropped, Range::all(), Range(focusLeftCol, focusRightCol));
  Mat left = Mat(cropped, Range::all(), Range(0, focusLeftCol));
  Mat right = Mat(cropped, Range::all(), Range(focusRightCol, cropped.cols));

  Size origSize(left.size());

  Mat blurredLeft = left;
  Mat blurredRight = right;

  Size smallSize(left.cols / BLUR_FACTOR, left.rows / BLUR_FACTOR);
  cv::resize(left, blurredLeft, smallSize);
  cv::resize(right, blurredRight, smallSize);

  OptimizedImage optImage(focused, blurredLeft, blurredRight, origSize, image.size(), image.type(), leftCol);
  return optImage;
}

Mat Optimizer::extractImage(const OptimizedImage& optImage) {
  Mat left, right;
  cv::resize(optImage.blurredLeft, left, optImage.origSize);
  cv::resize(optImage.blurredRight, right, optImage.origSize);

  // reconstruct cropped image
  Mat croppedImage;
  ImageUtil::hconcat3(left, optImage.focused, right, croppedImage);

  Mat fullLeft, fullCenter, fullRight;

  int numRows = croppedImage.size().height;

  // Split into 2 cases. In either case, there are 3 images to create
  if (croppedImage.size().width + optImage.leftBuffer >= optImage.fullSize.width) {
    // cropped image wraps around
    // Reconstruct as cropped_right + black + cropped_left

    // cropped right = leftBuffer to end, or in local coords, 0 to width - leftBuffer?
    // cropped left =
    // black = full width - right - left

    int rightEndExclusive = optImage.fullSize.width - optImage.leftBuffer;
    ASSERT(0 <= rightEndExclusive && rightEndExclusive <= croppedImage.cols);
    fullRight = Mat(croppedImage, Range::all(), Range(0, rightEndExclusive));
    fullLeft = Mat(croppedImage, Range::all(), Range(rightEndExclusive, croppedImage.cols));

    int centerCols = optImage.fullSize.width - fullRight.size().width - fullLeft.size().width;
    ASSERT(centerCols >= 0);

    fullCenter = Mat(numRows, centerCols, optImage.origType);
    fullCenter.setTo(cv::Scalar(0,0,0));

  } else {
    // cropped image fully contained
    // Reconstruct as black_left + cropped + black_right

    int leftBufferCols = optImage.leftBuffer; //(optImage.fullSize.width - croppedImage.size().width)/2;
    fullLeft = Mat(numRows, leftBufferCols, optImage.origType);
    fullCenter = croppedImage;

    int rightBufferCols = optImage.fullSize.width - leftBufferCols - croppedImage.size().width;
    ASSERT(rightBufferCols >= 0);

    fullRight = Mat(numRows, rightBufferCols, optImage.origType);

    fullLeft.setTo(cv::Scalar(0,0,0));
    fullRight.setTo(cv::Scalar(0,0,0));
  }


  Mat fullImage;
  ImageUtil::hconcat3(fullLeft, fullCenter, fullRight, fullImage);

  ENSURES(fullImage.size() == optImage.fullSize);

  return fullImage;
}
