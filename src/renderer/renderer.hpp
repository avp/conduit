#ifndef RENDERER_RENDERER_H_
#define RENDERER_RENDERER_H_

#include <GL/glew.h>
#include <OVR_CAPI_0_5_0.h>
#include <OVR_CAPI_GL.h>
#include <SDL2/SDL.h>
#include <glm/glm.hpp>
#include <iostream>
#include <opencv2/highgui/highgui.hpp>
#include <unistd.h>

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

  private:
    SDL_Window* win;
    SDL_GLContext ctx;

    ovrHmd hmd;
    ovrSizei eyeRes[2];
    ovrEyeRenderDesc eyeDesc[2];
    ovrSizei fovSize;
    ovrGLTexture ovrTex[2];
    ovrGLTexture* prTargetTex;

    int windowWidth, windowHeight;
    int fovTexWidth, fovTexHeight;
    unsigned int fbo, fovTex, fovDepth;
    ovrGLConfig glCfg;
    unsigned int distortCaps, hmdCaps;

    unsigned int nextPow2(unsigned int x);
};

#endif
