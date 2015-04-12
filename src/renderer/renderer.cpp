#include "renderer.hpp"

Renderer::Renderer(int w, int h) {
  using std::cout;
  using std::cerr;
  using std::endl;

  cout << "Initializing OVR..." << endl;
  ovr_Initialize(0);

  cout << "Initializing SDL..." << endl;
  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);

  int x = SDL_WINDOWPOS_UNDEFINED;
  int y = SDL_WINDOWPOS_UNDEFINED;
  unsigned int flags = SDL_WINDOW_OPENGL;

  cout << "Creating window..." << endl;
  win = SDL_CreateWindow("Conduit", x, y, w, h, flags);
  if (!win) {
    cerr << "Failed to create window." << endl;
    exit(1);
  }

  cout << "Creating context..." << endl;
  ctx = SDL_GL_CreateContext(win);
  if (!ctx) {
    cerr << "Failed to create context." << endl;
    exit(1);
  }

  cout << "Initializing GLEW..." << endl;
  glewInit();

  hmd = ovrHmd_Create(0);
  if (!hmd) {
    cout << "Failed to open Oculus device, using debug mode." << endl;
    hmd = ovrHmd_CreateDebug(ovrHmd_DK2);
    if (!hmd) {
      cerr << "Failed to debug mode HMD." << endl;
      exit(1);
    }
  }

  cout << "HMD Initialized: " << hmd->ProductName << endl;

  // Resize window for correct resolution
  windowWidth = hmd->Resolution.w;
  windowHeight = hmd->Resolution.h;
  cout << "Resizing window to " << windowWidth << "x" << windowHeight << endl;
  SDL_SetWindowSize(win, windowWidth, windowHeight);
  SDL_SetWindowPosition(win, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
}
