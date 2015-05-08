#ifndef RENDERTEST_RENDERTEST_H_
#define RENDERTEST_RENDERTEST_H_

#include <iostream>
#include <math.h>
#include <stdlib.h>
//#include <unistd.h>

#include <opencv2/highgui/highgui.hpp>
#include <OVR_CAPI_0_5_0.h>

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

class RenderTest {
  public:
    static int renderTest(int argc, char **argv, cv::Mat& image);
};

#endif
