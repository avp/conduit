#include "cylinderwarp.hpp"

using cv::Mat;
using cv::Point2f;
using cv::Point2i;
using cv::Vec3b;

Mat CylinderWarp::cylinderWarp(const Mat& image) {
  int height = image.rows;
  int width = image.cols;

  Mat result(image.size(), image.type());

  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      Point2f curPos(x,y);
      curPos = warpPoint(curPos, width, height);

      Point2i topLeft((int) curPos.x, (int) curPos.y);
      if (topLeft.x < 0 ||
          topLeft.x > width - 2 ||
          topLeft.y < 0 ||
          topLeft.y > height - 2) {
        continue;
      }

      float dx = curPos.x - topLeft.x;
      float dy = curPos.y - topLeft.y;

      float tl = (1.0 - dx) * (1.0 - dy);
      float tr = (dx) * (1.0 - dy);
      float bl = (1.0 - dx) * (dy);
      float br = (dx) * (dy);

      Vec3b value = tl * image.at<Vec3b>(topLeft) +
        tr * image.at<Vec3b>(topLeft.y, topLeft.x + 1) +
        bl * image.at<Vec3b>(topLeft.y + 1, topLeft.x) +
        br * image.at<Vec3b>(topLeft.y + 1, topLeft.x + 1);

      result.at<Vec3b>(y,x) = value;
    }
  }

  return result;
}

Point2f CylinderWarp::warpPoint(Point2f point, int width, int height) {
  Point2f pc(point.x - width / 2, point.y - height / 2);

  float f = width;
  float r = width;
  float om = width / 2; // omega (the angle)

  float z0 = f - sqrt(r * r - om * om);
  float zc = (2 * z0 + sqrt(4 * z0 * z0-4 * (pc.x * pc.x / (f * f) + 1) *
        (z0 * z0 - r * r))) / (2* (pc.x*pc.x/(f*f)+1));

  Point2f result(pc.x * zc / f, pc.y * zc / f);
  result.x += width / 2;
  result.y += height / 2;

  return result;
}
