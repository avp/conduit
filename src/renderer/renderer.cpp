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

  ovrHmd_ConfigureTracking(hmd,
      ovrTrackingCap_Orientation |
      ovrTrackingCap_MagYawCorrection |
      ovrTrackingCap_Position,
      0);

  eyeRes[0] = ovrHmd_GetFovTextureSize(hmd, ovrEye_Left,
      hmd->DefaultEyeFov[0], 1.0);
  eyeRes[1] = ovrHmd_GetFovTextureSize(hmd, ovrEye_Right,
      hmd->DefaultEyeFov[1], 1.0);

  fovSize.w = eyeRes[0].w + eyeRes[1].w;
  fovSize.h = std::max(eyeRes[0].h, eyeRes[1].h);

  fovTexWidth = nextPow2(fovSize.w);
  fovTexHeight = nextPow2(fovSize.h);

  for (int i = 0; i < 2; i++) {
    ovrTex[i].OGL.Header.API = ovrRenderAPI_OpenGL;
    ovrTex[i].OGL.Header.TextureSize.w = fovTexWidth;
    ovrTex[i].OGL.Header.TextureSize.h = fovTexHeight;
    /* this next field is the only one that differs between the two eyes */
    ovrTex[i].OGL.Header.RenderViewport.Pos.x = i == 0 ? 0 : fovSize.w / 2.0;
    ovrTex[i].OGL.Header.RenderViewport.Pos.y = 0;
    ovrTex[i].OGL.Header.RenderViewport.Size.w = fovSize.w / 2.0;
    ovrTex[i].OGL.Header.RenderViewport.Size.h = fovSize.h;
    ovrTex[i].OGL.TexId = fovTex; /* both eyes will use the same texture id */
  }

  memset(&glCfg, 0, sizeof(glCfg));
  glCfg.OGL.Header.API = ovrRenderAPI_OpenGL;
  glCfg.OGL.Header.BackBufferSize = hmd->Resolution;
  glCfg.OGL.Header.Multisample = 1;

#ifdef OVR_OS_LINUX
  glCfg.OGL.Disp = glXGetCurrentDisplay();
#endif

  cout << "Running in direct-hmd mode." << endl;

#ifdef OVR_OS_LINUX
  cout << "Linux: Attaching to window..." << endl;
  ovrHmd_AttachToWindow(hmd, (void*) glXGetCurrentDrawable(), 0, 0);
  cout << "Linux: Attached to window." << endl;
#endif

  hmdCaps = ovrHmdCap_LowPersistence | ovrHmdCap_DynamicPrediction;
  ovrHmd_SetEnabledCaps(hmd, hmdCaps);

  cout << "Configuring distortion rendering..." << endl;
  distortCaps = ovrDistortionCap_TimeWarp | ovrDistortionCap_Overdrive;
  if (!ovrHmd_ConfigureRendering(hmd, &glCfg.Config, distortCaps,
        hmd->DefaultEyeFov, eyeDesc)) {
    cerr << "Failed to configure distortion rendering." << endl;
    exit(1);
  }
  cout << "Configured distortion rendering." << endl;

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glEnable(GL_LIGHT1);
  glEnable(GL_NORMALIZE);

  glClearColor(0.1, 0.1, 0.1, 1);
}

unsigned int Renderer::nextPow2(unsigned int x) {
  x--;
  x |= x >> 1;
  x |= x >> 2;
  x |= x >> 4;
  x |= x >> 8;
  x |= x >> 16;
  return x + 1;
}
