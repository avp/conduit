#include "imageutil.hpp"
#include "../contracts.h"

using cv::Mat;

void ImageUtil::glPixelsToMat(cv::Mat& image) {
  int width = image.cols;
  int height = image.rows;

  glPixelStorei(GL_PACK_ALIGNMENT, (image.step & 3) ? 1 : 4);
  glPixelStorei(GL_PACK_ROW_LENGTH, image.step / image.elemSize());

  glReadPixels(0, 0, width, height, GL_BGR, GL_UNSIGNED_BYTE, image.ptr());

  cv::Mat flipped;
  cv::flip(image, flipped, 0);
  image = flipped;
}

size_t ImageUtil::imageSize(const cv::Mat& image) {
  return image.total() * image.elemSize();
}

void ImageUtil::hconcat3(const Mat& m1, const Mat& m2, const Mat& m3,
    Mat& dst) {
  int num_empty = 0;
  if (m1.empty()) num_empty++;
  if (m2.empty()) num_empty++;
  if (m3.empty()) num_empty++;
  REQUIRES(num_empty < 3);

  std::vector<Mat> mats;
  if (!m1.empty())
    mats.push_back(m1);
  if (!m2.empty())
    mats.push_back(m2);
  if (!m3.empty())
    mats.push_back(m3);
  hconcat(mats, dst);
}

void ImageUtil::vconcat3(const Mat& m1, const Mat& m2, const Mat& m3,
    Mat& dst) {
  REQUIRES(!m1.empty());
  REQUIRES(!m2.empty());
  REQUIRES(!m3.empty());

  std::vector<Mat> mats;
  mats.push_back(m1);
  mats.push_back(m2);
  mats.push_back(m3);
  vconcat(mats, dst);
}
