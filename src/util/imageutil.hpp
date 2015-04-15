#ifndef UTIL_IMAGEUTIL_H_
#define UTIL_IMAGEUTIL_H_

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glut.h>
#include <glm/glm.hpp>

#include <opencv2/highgui/highgui.hpp>

class ImageUtil {
  public:
    ImageUtil();
    static void glPixelsToMat(cv::Mat& image);
};

#endif
