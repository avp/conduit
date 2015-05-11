#include "optimizer.hpp"

#include <iostream>

using std::vector;

using cv::Mat;
using cv::Range;
using cv::Size;

int BLUR_FACTOR = BLUR_NORMAL;

FrameData::FrameData(const cv::Mat& image, double timestamp) {
  this->image = image;
  this->timestamp = timestamp;
}

OptimizedImage::OptimizedImage(const cv::Mat& focusedTop,
    const cv::Mat& focusedBot,
    const cv::Mat& blurred,
    int focusRow, int focusCol,
    Size croppedSize, Size fullSize, int leftBuffer) {
  this->focusedTop = Mat(focusedTop);
  this->focusedBot = Mat(focusedBot);
  this->blurred = Mat(blurred);
  this->focusRow = focusRow;
  this->focusCol = focusCol;
  this->croppedSize = croppedSize;
  this->fullSize = fullSize;
  this->leftBuffer = leftBuffer;
}

size_t OptimizedImage::size() const {
  return ImageUtil::imageSize(focusedTop) +
    ImageUtil::imageSize(focusedBot) +
    ImageUtil::imageSize(blurred);
}

static inline int constrainAngle(int x) {
  x %= 360;
  if (x < 0)
    x += 360;
  ENSURES(0 <= x && x < 360);
  return x;
}

static inline int clamp(int x, int lo, int hi) {
  return std::max(lo, std::min(x, hi));
}

static Mat cropHorizontallyWrapped(const Mat& image, const int leftCol, const int rightCol) {
  REQUIRES(0 <= leftCol && leftCol < image.cols);
  REQUIRES(0 <= rightCol && rightCol < image.cols);

  Mat cropped;
  if (leftCol < rightCol) {
    // Cropped window doesn't wrap around, simple case.
    cropped = Mat(image, Range::all(), Range(leftCol, rightCol));
  } else {
    ASSERT(leftCol > rightCol);
    // Cropped window *does* wrap around. We need to get the part
    // before and after it wraps, which are in two separate places
    // on the image.
    Mat leftMat = Mat(image, Range::all(), Range(leftCol, image.cols));
    Mat rightMat = Mat(image, Range::all(), Range(0, rightCol));
    ImageUtil::hconcat2(leftMat, rightMat, cropped);
  }
  return cropped;
}

OptimizedImage Optimizer::optimizeImage(const Mat& image,
    int angle, int vAngle) {
  Timer timer;

  angle = constrainAngle(angle);
  REQUIRES(H_FOCUS_ANGLE < CROP_ANGLE);

  const int width = image.cols;
  const int height = image.rows;
  const double angleToWidth = width / 360.0;
  const double angleToHeight = height / 2.0 / 180.0;

  int leftAngle = constrainAngle(angle - CROP_ANGLE / 2);
  int rightAngle = constrainAngle(angle + CROP_ANGLE / 2);

  int leftCol = leftAngle * angleToWidth;
  int rightCol = rightAngle * angleToWidth;
  ASSERT(0 <= leftCol);
  ASSERT(leftCol < width);
  ASSERT(0 <= rightCol);
  ASSERT(rightCol < width);

  timer.start();
  Mat cropped = cropHorizontallyWrapped(image, leftCol, rightCol);
  timer.stop("Cropping");

  int focusWidth = H_FOCUS_ANGLE * angleToWidth;

  int focusLeftCol = cropped.cols / 2 - focusWidth / 2;
  int focusRightCol = cropped.cols / 2 + focusWidth / 2;

  timer.start();
  ASSERT(0 <= focusLeftCol);
  ASSERT(focusLeftCol <= focusRightCol);
  ASSERT(focusRightCol < cropped.cols);
  Mat middle = Mat(cropped, Range::all(), Range(focusLeftCol, focusRightCol));
  timer.stop("Splitting (H)");

  int focusHeight = V_FOCUS_ANGLE * angleToHeight;
  int focusMiddleRow = clamp(vAngle * angleToHeight,
      focusHeight / 2, (image.rows / 2) - focusHeight / 2);
  int focusTopRow = focusMiddleRow - focusHeight / 2;
  int focusBottomRow = focusMiddleRow + focusHeight / 2;

  timer.start();
  Mat focusedTop(middle,
      Range(focusTopRow, focusBottomRow));
  Mat focusedBot(middle,
      Range(focusTopRow + height / 2, focusBottomRow + height / 2));
  timer.stop("Splitting (V)");

  Size smallSize(cropped.cols / BLUR_FACTOR, cropped.rows / BLUR_FACTOR);

  timer.start();
  Mat blurred;
  cv::resize(cropped, blurred, smallSize);
  timer.stop("Blurring");

  OptimizedImage optImage(focusedTop, focusedBot, blurred,
      focusTopRow, focusLeftCol,
      cropped.size(), image.size(), leftCol);
  return optImage;
}

static Mat uncropWrapped(const Mat& croppedImage, const int fullWidth, const int leftBuffer) {
  int numRows = croppedImage.size().height;
  int origType = croppedImage.type(); // we use the same type everywhere

  Mat fullLeft, fullCenter, fullRight;

  // Split into 2 cases. In either case, there are 3 images to create
  if (croppedImage.size().width + leftBuffer >= fullWidth) {
    // cropped image wraps around
    // Reconstruct as cropped_right + black + cropped_left

    // cropped right = leftBuffer to end, or in local coords, 0 to width - leftBuffer?
    // cropped left =
    // black = full width - right - left

    int rightEndExclusive = fullWidth - leftBuffer;
    ASSERT(0 <= rightEndExclusive && rightEndExclusive <= croppedImage.cols);
    fullRight = Mat(croppedImage, Range::all(), Range(0, rightEndExclusive));
    fullLeft = Mat(croppedImage, Range::all(), Range(rightEndExclusive, croppedImage.cols));

    int centerCols = fullWidth - fullRight.size().width - fullLeft.size().width;
    ASSERT(centerCols >= 0);

    fullCenter = Mat(numRows, centerCols, origType);
    fullCenter.setTo(cv::Scalar(0,0,0));

  } else {
    // cropped image fully contained
    // Reconstruct as black_left + cropped + black_right

    fullLeft = Mat(numRows, leftBuffer, origType);
    fullCenter = croppedImage;

    int rightBufferCols = fullWidth - leftBuffer - croppedImage.size().width;
    ASSERT(rightBufferCols >= 0);

    fullRight = Mat(numRows, rightBufferCols, origType);

    fullLeft.setTo(cv::Scalar(0,0,0));
    fullRight.setTo(cv::Scalar(0,0,0));
  }


  Mat fullImage;
  ImageUtil::hconcat3(fullLeft, fullCenter, fullRight, fullImage);
  return fullImage;
}

bool FOVEA_DISPLAY = true;

Mat Optimizer::extractImage(const OptimizedImage& optImage) {
  Timer timer;

  Mat croppedImage;

  timer.start();
  cv::resize(optImage.blurred, croppedImage, optImage.croppedSize);
  timer.stop("Expanding");

  if (FOVEA_DISPLAY) {
    Mat tmp;
    timer.start();
    tmp = croppedImage(
        Range(optImage.focusRow, optImage.focusRow + optImage.focusedTop.rows),
        Range(optImage.focusCol, optImage.focusCol + optImage.focusedTop.cols));
    optImage.focusedTop.copyTo(tmp);
    tmp = croppedImage(
        Range(optImage.focusRow + croppedImage.rows / 2,
          optImage.focusRow + croppedImage.rows / 2 + optImage.focusedBot.rows),
        Range(optImage.focusCol, optImage.focusCol + optImage.focusedBot.cols));
    optImage.focusedBot.copyTo(tmp);
    timer.stop("Reconstructing");
  }
  
  // Reconstruct middle image
  Mat fullLeft, fullCenter, fullRight;

  timer.start();
  Mat fullImage = uncropWrapped(croppedImage, optImage.fullSize.width, optImage.leftBuffer);
  timer.stop("Full image");

  ENSURES(fullImage.size() == optImage.fullSize);

  return fullImage;
}

cv::Mat Optimizer::processImage(const cv::Mat& input,
        int angle, int vAngle) {
  OptimizedImage opt = optimizeImage(input, angle, vAngle);
  return extractImage(opt);
}

// OptimizerPipeline

OptimizerPipeline::OptimizerPipeline(VideoReader* vr) {
  pthread_cond_init(&queueCond, NULL);
  pthread_mutex_init(&queueLock, NULL);

  bufferThread = std::thread(&OptimizerPipeline::bufferFrames, this, vr);
  bufferThread.detach();
}

int OptimizerPipeline::getNumFramesAvailable() {
  return frameQueue.size();
}

FrameData OptimizerPipeline::getFrame() {
  FrameData frame = frameQueue.dequeue();
  pthread_cond_signal(&queueCond);
  return frame;
}

void OptimizerPipeline::bufferFrames(VideoReader* vr) {
  while (true) {
    if (frameQueue.size() > OPTIMIZER_QUEUE_SIZE) {
      pthread_mutex_lock(&queueLock);
      pthread_cond_wait(&queueCond, &queueLock);
      pthread_mutex_unlock(&queueLock);
    }
    cv::Mat frame = vr->getFrame();

    // important to check isDone before changing frame, or it'll be different!
    if (frame.empty()) {
      fullyBuffered = true;
      return;
    }

    hmdDataMutex.lock();
    int hAngleCached = hAngle;
    int vAngleCached = vAngle;
    double lastUpdatedCached = lastUpdated;
    hmdDataMutex.unlock();

    frame = Optimizer::processImage(frame, hAngleCached, vAngleCached);

    FrameData fd(frame, lastUpdatedCached);

    frameQueue.enqueue(fd);
    frameAvailable = true;
  }
}

bool OptimizerPipeline::isFrameAvailable() {
  if (!frameAvailable) return false;
  if (frameQueue.size() > 0)
    return true;
  else {
    frameAvailable = false;
    return false;
  }
}
