#ifndef RENDERER_RENDERER_H_
#define RENDERER_RENDERER_H_

#include <GL/glew.h>
#include <OVR_CAPI.h>
#include <SDL2/SDL.h>
#include <glm/glm.hpp>
#include <iostream>
#include <opencv2/highgui/highgui.hpp>
#include <unistd.h>

class Renderer {
  public:
    Renderer(int w, int h);

  private:
    SDL_Window* win;
    SDL_GLContext ctx;

    ovrHmd hmd;

    int windowWidth;
    int windowHeight;
};

#endif
