#ifndef RENDERTEST_RENDERTEST_H_
#define RENDERTEST_RENDERTEST_H_

#include <stdlib.h>
#include <math.h>

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

class RenderTest {
  public:
    static int renderTest(int argc, char **argv);
};

#endif
