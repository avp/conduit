#include "imageutil.hpp"

void ImageUtil::glPixelsToMat(cv::Mat& image) {
  int width = image.cols;
  int height = image.rows;

  glPixelStorei(GL_PACK_ALIGNMENT, (image.step & 3) ? 1 : 4);
  glPixelStorei(GL_PACK_ROW_LENGTH, image.step / image.elemSize());

  glReadPixels(0, 0, width, height, GL_BGR, GL_UNSIGNED_BYTE, image.data);

  cv::Mat flipped;
  cv::flip(image, flipped, 0);

  image = flipped;
}
