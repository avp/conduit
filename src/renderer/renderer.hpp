#ifndef RENDERER_RENDERER_H_
#define RENDERER_RENDERER_H_

#include <iostream>
#include <unistd.h>

#include <GL/glew.h>

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <GLUT/glut.h>
#else
#include <GL/gl.h>
#include <GL/glut.h>
#endif

#include <glm/glm.hpp>

#include <OVR_CAPI_0_5_0.h>
#include <OVR_CAPI_GL.h>

#include <SDL2/SDL.h>
#include <opencv2/highgui/highgui.hpp>

#include "../util/imageutil.hpp"

#ifdef WIN32
#define OVR_OS_WIN32

#elif defined(__APPLE__)
#define OVR_OS_MAC

#else
#define OVR_OS_LINUX
#include <GL/glx.h>
#include <X11/Xlib.h>

#endif

class Renderer {
  public:
    Renderer(int w, int h);
    void displayStereoImage(const cv::Mat& image);

  private:
    SDL_Window* win;
    SDL_GLContext ctx;

    ovrHmd hmd;
    ovrSizei eyeRes[2];
    ovrEyeRenderDesc eyeDesc[2];
    ovrSizei fovSize;
    ovrGLTexture ovrTex[2];
    ovrGLTexture* prTargetTex;

    GLuint texture[2];

    int windowWidth, windowHeight;
    int fbTexWidth, fbTexHeight;
    unsigned int fbo, fbTex, fbDepth;
    ovrGLConfig glCfg;
    unsigned int distortCaps, hmdCaps;

    void updateRenderTarget();
    static GLuint loadTexture(const cv::Mat& image);
    static unsigned int nextPow2(unsigned int x);
};

#endif
