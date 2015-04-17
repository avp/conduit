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

  // cout << "Configuring distortion rendering..." << endl;
  // distortCaps = ovrDistortionCap_TimeWarp | ovrDistortionCap_Overdrive;
  // if (!ovrHmd_ConfigureRendering(hmd, &glCfg.Config, distortCaps,
  //       hmd->DefaultEyeFov, eyeDesc)) {
  //   cerr << "Failed to configure distortion rendering." << endl;
  //   exit(1);
  // }
  // cout << "Configured distortion rendering." << endl;

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glEnable(GL_NORMALIZE);

  glClearColor(0.1, 0.1, 0.1, 1);
}

GLuint Renderer::loadTexture(const cv::Mat& image) {
  int height = image.rows;
  int width = image.cols;

  GLuint texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);

  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

  // build our texture
  // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0,
  //     GL_BGR, GL_UNSIGNED_BYTE, image.ptr());
  gluBuild2DMipmaps(GL_TEXTURE_2D, 3, width, height,
      GL_BGR, GL_UNSIGNED_BYTE, image.ptr());

  glBindTexture(GL_TEXTURE_2D, 0);
  return texture;
}

void display() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glTranslatef(0.0f, 0.0f, -5.0f);

  glLoadIdentity();

  glBegin(GL_QUADS);
  glColor3ub(255, 0, 0);
  glVertex2f(10, 10);
  glVertex2f(400, 10);
  glVertex2f(400, 400);
  glVertex2f(10, 400);
  glEnd();

  glFlush();
}

void dummy() {
  std::cout << "Redisplay\n";
}

void Renderer::displayStereoImage(const cv::Mat& image) {
  int width = image.cols;
  int height = image.rows / 2;

  cv::Mat images[2];
  cv::Mat results[2];
  GLuint textures[2];

  glDisable(GL_LIGHTING);
  glDisable(GL_BLEND);

  glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
  glutInitWindowSize(width, height);
  glutCreateWindow("GL Window");
  glutDisplayFunc(dummy);

  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0.0, width - 1, height - 1, 0, -1.0, 1.0);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();

  glEnable(GL_TEXTURE_2D);

  for (int i = 0; i < 2; i++) {
    images[i] = cv::Mat(image, cv::Range(height * i, height * (i + 1)));
    textures[i] = loadTexture(images[i]);

    // Bind the texture so it gets used
    glBindTexture(GL_TEXTURE_2D, textures[i]);

    // Draw and texture the rectangle
    glClearColor(0, 0, 0, 0);
    glBegin(GL_QUADS);

    glTexCoord2f(0.0, 0.0);
    glVertex3f(0, 0, 0);

    glTexCoord2f(0.0, 1.0);
    glVertex3f(0, height, 0);

    glTexCoord2f(1.0, 1.0);
    glVertex3f(width, height, 0);

    glTexCoord2f(1.0, 0.0);
    glVertex3f(width, 0, 0);

    glEnd();

    glBindTexture(GL_TEXTURE_2D, 0);
    glFlush();

    results[i] = cv::Mat(height, width, CV_8UC3);
    std::cout << width << "x" << height << std::endl;
    ImageUtil::glPixelsToMat(results[i]);
  }

  glDisable(GL_TEXTURE_2D);

  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);

  cv::namedWindow("right", CV_WINDOW_NORMAL);
  imshow("right", results[1]);

  cv::waitKey(5000);
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
