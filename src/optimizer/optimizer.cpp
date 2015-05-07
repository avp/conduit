#include "optimizer.hpp"

#include <iostream>
#include <math.h>

using std::vector;

using cv::Mat;
using cv::Range;
using cv::Size;

static const int CROP_ANGLE = 120;
static const int H_FOCUS_ANGLE = 20;
static const int V_FOCUS_ANGLE = 20;
static const int BLUR_FACTOR = 5;

OptimizedImage::OptimizedImage(const Mat& focused,
    const Mat& blurredLeft,
    const Mat& blurredRight,
    const Mat& blurredTop,
    const Mat& blurredBottom,
    Size origHSize,
    Size origVSize,
    Size fullSize,
    int leftBuffer) {
  this->focused = Mat(focused);
  this->blurredLeft = Mat(blurredLeft);
  this->blurredRight = Mat(blurredRight);
  this->blurredTop = Mat(blurredTop);
  this->blurredBottom = Mat(blurredBottom);
  this->origHSize = origHSize;
  this->origVSize = origVSize;
  this->fullSize = fullSize;
  this->leftBuffer = leftBuffer;
}

size_t OptimizedImage::size() const {
  return ImageUtil::imageSize(focused) +
    ImageUtil::imageSize(blurredLeft) +
    ImageUtil::imageSize(blurredRight) +
    ImageUtil::imageSize(blurredTop) +
    ImageUtil::imageSize(blurredBottom);
}

int constrainAngle(int x){
  x = x % 360;
  if (x < 0)
    x += 360;
  ENSURES(0 <= x && x < 360);
  return x;
}

double constrainAngle(double x){
  // https://stackoverflow.com/questions/11498169/dealing-with-angle-wrap-in-c-code
  x = fmod(x,360);
  if (x < 0)
    x += 360;
  ENSURES(0 <= x && x < 360);
  return x;
}

OptimizedImage Optimizer::optimizeImage(const Mat& image, int angle) {
  angle = constrainAngle(angle);
  REQUIRES(H_FOCUS_ANGLE < CROP_ANGLE);

  const int width = image.cols;
  const double angleToWidth = width / 360.0;
  const double angleToHeight = width / 180.0;

  int leftAngle = constrainAngle(angle - CROP_ANGLE / 2);
  int rightAngle = constrainAngle(angle + CROP_ANGLE / 2);

  int leftCol = leftAngle * angleToWidth;
  int rightCol = rightAngle * angleToWidth;
  ASSERT(0 <= leftCol);
  ASSERT(leftCol < width);
  ASSERT(0 <= rightCol);
  ASSERT(rightCol < width);

  Mat cropped;
  if (leftCol < rightCol) {
    // Cropped window doesn't wrap around, simple case.
    cropped = Mat(image, Range::all(), Range(leftCol, rightCol));
  } else {
    // Cropped window *does* wrap around. We need to get the part
    // before and after it wraps, which are in two separate places
    // on the image.
    Mat leftMat = Mat(image, Range::all(), Range(leftCol, width));
    Mat rightMat = Mat(image, Range::all(), Range(0, rightCol));
    hconcat(leftMat, rightMat, cropped);
  }

  int focusWidth = H_FOCUS_ANGLE * angleToWidth;

  int focusLeftCol = cropped.cols / 2 - focusWidth / 2;
  int focusRightCol = cropped.cols / 2 + focusWidth / 2;

  ASSERT(0 <= focusLeftCol);
  ASSERT(focusLeftCol <= focusRightCol);
  ASSERT(focusRightCol < cropped.cols);
  Mat focused = Mat(cropped, Range::all(), Range(focusLeftCol, focusRightCol));
  Mat left = Mat(cropped, Range::all(), Range(0, focusLeftCol));
  Mat right = Mat(cropped, Range::all(), Range(focusRightCol, cropped.cols));

  Size origHSize(left.size());

  Mat blurredLeft = left;
  Mat blurredRight = right;

  Size smallSize(left.cols / BLUR_FACTOR, left.rows / BLUR_FACTOR);
  cv::resize(left, blurredLeft, smallSize);
  cv::resize(right, blurredRight, smallSize);

  int focusHeight = V_FOCUS_ANGLE * angleToHeight;
  int focusTopCol = focused.rows / 2 - focusHeight / 2;
  int focusBottomCol = focused.rows / 2 + focusHeight / 2;

  Mat top(cropped, Range(0, focusTopCol));
  Mat center(cropped, Range(focusTopCol, focusBottomCol));
  Mat bottom(cropped, Range(focusBottomCol, focused.rows));

  Mat blurredTop, blurredBottom;
  Size origVSize(top.size());
  Size smallVSize(top.cols / BLUR_FACTOR, top.rows / BLUR_FACTOR);
  cv::resize(top, blurredTop, smallVSize);
  cv::resize(bottom, blurredBottom, smallVSize);

  OptimizedImage optImage(center,
      blurredLeft, blurredRight,
      blurredTop, blurredBottom,
      origHSize, origVSize,
      image.size(), leftCol);
  return optImage;
}

Mat Optimizer::extractImage(const OptimizedImage& optImage) {
  Mat left, right, top, bottom;

  cv::resize(optImage.blurredLeft, left, optImage.origHSize);
  cv::resize(optImage.blurredRight, right, optImage.origHSize);

  cv::resize(optImage.blurredTop, top, optImage.origVSize);
  cv::resize(optImage.blurredBottom, bottom, optImage.origVSize);

  // Reconstruct middle image
  Mat middle;
  ImageUtil::vconcat3(top, optImage.focused, bottom, middle);

  // Reconstruct cropped image
  Mat croppedImage;
  ImageUtil::hconcat3(left, middle, right, croppedImage);

  Mat fullLeft, fullCenter, fullRight;

  int numRows = croppedImage.size().height;

  int origType = optImage.focused.type();

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

    fullCenter = Mat(numRows, centerCols, origType);
    fullCenter.setTo(cv::Scalar(0,0,0));

  } else {
    // cropped image fully contained
    // Reconstruct as black_left + cropped + black_right

    int leftBufferCols = optImage.leftBuffer; //(optImage.fullSize.width - croppedImage.size().width)/2;
    fullLeft = Mat(numRows, leftBufferCols, origType);
    fullCenter = croppedImage;

    int rightBufferCols = optImage.fullSize.width - leftBufferCols - croppedImage.size().width;
    ASSERT(rightBufferCols >= 0);

    fullRight = Mat(numRows, rightBufferCols, origType);

    fullLeft.setTo(cv::Scalar(0,0,0));
    fullRight.setTo(cv::Scalar(0,0,0));
  }


  Mat fullImage;
  ImageUtil::hconcat3(fullLeft, fullCenter, fullRight, fullImage);

  ENSURES(fullImage.size() == optImage.fullSize);

  return fullImage;
}
