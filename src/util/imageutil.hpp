#ifndef UTIL_IMAGEUTIL_H_
#define UTIL_IMAGEUTIL_H_

#include <GL/glew.h>

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <GLUT/glut.h>
#else
#include <GL/gl.h>
#include <GL/glut.h>
#endif

#include <glm/glm.hpp>

#include <opencv2/highgui/highgui.hpp>

class ImageUtil {
  public:
    static void glPixelsToMat(cv::Mat& image);
    static size_t imageSize(const cv::Mat& image);
};

#endif
