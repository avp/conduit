/* Very simple OculusSDK OpenGL usage example.
 *
 * Uses SDL2 (www.libsdl.org) for event handling and OpenGL context management.
 * Uses GLEW (glew.sourceforge.net) for OpenGL extension wrangling.
 *
 * Author: John Tsiombikas <nuclear@member.fsf.org>
 * This code is in the public domain. Do whatever you like with it.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <SDL2/SDL.h>
#include <GL/glew.h>

#ifdef WIN32
#define OVR_OS_WIN32
#elif defined(__APPLE__)
#define OVR_OS_MAC
#else
#define OVR_OS_LINUX
#include <X11/Xlib.h>
#include <GL/glx.h>
#endif

#include <OVR_CAPI.h>
#include <OVR_CAPI_GL.h>

#include "oculus2.hpp"
#include "../videoreader/videoreader.hpp"

static int init(void);
static void cleanup(void);
static void toggle_hmd_fullscreen(void);
static void display(void);
static void draw_scene(void);
static void draw_box(float xsz, float ysz, float zsz, float norm_sign);
static void update_rtarg(int width, int height);
static int handle_event(SDL_Event *ev);
static int key_event(int key, int state);
static void reshape(int x, int y);
static unsigned int next_pow2(unsigned int x);
static void quat_to_matrix(const float *quat, float *mat);
static unsigned int gen_chess_tex(float r0, float g0, float b0, float r1, float g1, float b1);

static SDL_Window *win;
static SDL_GLContext ctx;
static int win_width, win_height;

static unsigned int fbo, fb_tex, fb_depth;
static int fb_width, fb_height;
static int fb_tex_width, fb_tex_height;

static ovrHmd hmd;
static ovrSizei eyeres[2];
static ovrEyeRenderDesc eye_rdesc[2];
static ovrGLTexture fb_ovr_tex[2];
static union ovrGLConfig glcfg;
static unsigned int distort_caps;
static unsigned int hmd_caps;

static unsigned int chess_tex;

static GLUquadric* qobj;

static float xPos = 0, yPos = 0, zPos = 0, ourAngle = 0;

static GLuint videoTexture;

static GLuint loadTexture(const cv::Mat& image) {
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


int Oculus2::main(int argc, char **argv)
{
	if (argc < 3) {
		std::cerr << "Usage: "
		 << argv[0]
		 << " rendertest filename"
		 << std::endl;
		return 1;
	}

	if(init() == -1) {
		return 1;
	}

	std::string filename = argv[2];
	VideoReader videoReader(filename);
	cv::Mat image = videoReader.getFrame();
	cv::Mat left = cv::Mat(image, cv::Range(0, image.rows / 2));
	videoTexture = loadTexture(left);

	for(;;) {
		SDL_Event ev;
		while(SDL_PollEvent(&ev)) {
			if(handle_event(&ev) == -1) {
				goto done;
			}
		}
		display();
	}

done:
	cleanup();
	return 0;
}


int init(void)
{
	int i, x, y;
	unsigned int flags;

	/* libovr must be initialized before we create the OpenGL context */
	ovr_Initialize(0);

	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);

	x = y = SDL_WINDOWPOS_UNDEFINED;
	flags = SDL_WINDOW_OPENGL;
	if(!(win = SDL_CreateWindow("press 'f' to move to the HMD", x, y, 1024, 640, flags))) {
		fprintf(stderr, "failed to create window\n");
		return -1;
	}
	if(!(ctx = SDL_GL_CreateContext(win))) {
		fprintf(stderr, "failed to create OpenGL context\n");
		return -1;
	}

	glewInit();

	bool is_debug = false;
	if(!(hmd = ovrHmd_Create(0))) {
		fprintf(stderr, "failed to open Oculus HMD, falling back to virtual debug HMD\n");
		is_debug = true;
		if(!(hmd = ovrHmd_CreateDebug(ovrHmd_DK2))) {
			fprintf(stderr, "failed to create virtual debug HMD\n");
			return -1;
		}
	}
	printf("initialized HMD: %s - %s\n", hmd->Manufacturer, hmd->ProductName);

	/* resize our window to match the HMD resolution */
	win_width = hmd->Resolution.w;
	win_height = hmd->Resolution.h;
	if (is_debug) {
		win_width /= 2;
		win_height /= 2;
	}

	SDL_SetWindowSize(win, win_width, win_height);
	SDL_SetWindowPosition(win, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

	/* enable position and rotation tracking */
	ovrHmd_ConfigureTracking(hmd, ovrTrackingCap_Orientation | ovrTrackingCap_MagYawCorrection | ovrTrackingCap_Position, 0);
	/* retrieve the optimal render target resolution for each eye */
	eyeres[0] = ovrHmd_GetFovTextureSize(hmd, ovrEye_Left, hmd->DefaultEyeFov[0], 1.0);
	eyeres[1] = ovrHmd_GetFovTextureSize(hmd, ovrEye_Right, hmd->DefaultEyeFov[1], 1.0);

	/* and create a single render target texture to encompass both eyes */
	fb_width = eyeres[0].w + eyeres[1].w;
	fb_height = eyeres[0].h > eyeres[1].h ? eyeres[0].h : eyeres[1].h;
	update_rtarg(fb_width, fb_height);

	/* fill in the ovrGLTexture structures that describe our render target texture */
	for(i=0; i<2; i++) {
		fb_ovr_tex[i].OGL.Header.API = ovrRenderAPI_OpenGL;
		fb_ovr_tex[i].OGL.Header.TextureSize.w = fb_tex_width;
		fb_ovr_tex[i].OGL.Header.TextureSize.h = fb_tex_height;
		/* this next field is the only one that differs between the two eyes */
		fb_ovr_tex[i].OGL.Header.RenderViewport.Pos.x = i == 0 ? 0 : fb_width / 2.0;
		fb_ovr_tex[i].OGL.Header.RenderViewport.Pos.y = 0;
		fb_ovr_tex[i].OGL.Header.RenderViewport.Size.w = fb_width / 2.0;
		fb_ovr_tex[i].OGL.Header.RenderViewport.Size.h = fb_height;
		fb_ovr_tex[i].OGL.TexId = fb_tex;	/* both eyes will use the same texture id */
	}

	/* fill in the ovrGLConfig structure needed by the SDK to draw our stereo pair
	 * to the actual HMD display (SDK-distortion mode)
	 */
	memset(&glcfg, 0, sizeof glcfg);
	glcfg.OGL.Header.API = ovrRenderAPI_OpenGL;
	glcfg.OGL.Header.BackBufferSize.w = win_width;
	glcfg.OGL.Header.BackBufferSize.h = win_height;
	glcfg.OGL.Header.Multisample = 1;

#ifdef OVR_OS_WIN32
	glcfg.OGL.Window = GetActiveWindow();
	glcfg.OGL.DC = wglGetCurrentDC();
#elif defined(OVR_OS_LINUX)
	glcfg.OGL.Disp = glXGetCurrentDisplay();
#endif

	if(hmd->HmdCaps & ovrHmdCap_ExtendDesktop) {
		printf("running in \"extended desktop\" mode\n");
	} else {
		/* to sucessfully draw to the HMD display in "direct-hmd" mode, we have to
		 * call ovrHmd_AttachToWindow
		 * XXX: this doesn't work properly yet due to bugs in the oculus 0.4.1 sdk/driver
		 */
#ifdef WIN32
		ovrHmd_AttachToWindow(hmd, glcfg.OGL.Window, 0, 0);
#elif defined(OVR_OS_LINUX)
		ovrHmd_AttachToWindow(hmd, (void*)glXGetCurrentDrawable(), 0, 0);
#endif
		printf("running in \"direct-hmd\" mode\n");
	}

	/* enable low-persistence display and dynamic prediction for lattency compensation */
	hmd_caps = ovrHmdCap_LowPersistence | ovrHmdCap_DynamicPrediction;
	ovrHmd_SetEnabledCaps(hmd, hmd_caps);

	/* configure SDK-rendering and enable OLED overdrive and timewrap, which
	 * shifts the image before drawing to counter any lattency between the call
	 * to ovrHmd_GetEyePose and ovrHmd_EndFrame.
	 */
	distort_caps = ovrDistortionCap_TimeWarp | ovrDistortionCap_Overdrive;
	if(!ovrHmd_ConfigureRendering(hmd, &glcfg.Config, distort_caps, hmd->DefaultEyeFov, eye_rdesc)) {
		fprintf(stderr, "failed to configure distortion renderer\n");
	}

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	// glEnable(GL_LIGHTING);
	// glEnable(GL_LIGHT0);
	// glEnable(GL_LIGHT1);
	glEnable(GL_NORMALIZE);

	glClearColor(0.1, 0.1, 0.1, 1);

	chess_tex = gen_chess_tex(1.0, 0.7, 0.4, 0.4, 0.7, 1.0);

	qobj = gluNewQuadric();
	gluQuadricNormals(qobj, GLU_SMOOTH);

	if (!is_debug)
		toggle_hmd_fullscreen();

	return 0;
}

void cleanup(void)
{
	printf("Cleaning up...\n");

	if(hmd) {
		ovrHmd_Destroy(hmd);
	}
	ovr_Shutdown();

	SDL_Quit();
}

void toggle_hmd_fullscreen(void)
{
	static int fullscr, prev_x, prev_y;
	fullscr = !fullscr;

	if(fullscr) {
		/* going fullscreen on the rift. save current window position, and move it
		 * to the rift's part of the desktop before going fullscreen
		 */
		SDL_GetWindowPosition(win, &prev_x, &prev_y);
		SDL_SetWindowPosition(win, hmd->WindowsPos.x, hmd->WindowsPos.y);
		SDL_SetWindowFullscreen(win, SDL_WINDOW_FULLSCREEN_DESKTOP);

#ifdef OVR_OS_LINUX
		/* on linux for now we have to deal with screen rotation during rendering. The docs are promoting
		 * not rotating the DK2 screen globally
		 */
		glcfg.OGL.Header.BackBufferSize.w = hmd->Resolution.h;
		glcfg.OGL.Header.BackBufferSize.h = hmd->Resolution.w;

		distort_caps |= ovrDistortionCap_LinuxDevFullscreen;
		ovrHmd_ConfigureRendering(hmd, &glcfg.Config, distort_caps, hmd->DefaultEyeFov, eye_rdesc);
#endif
	} else {
		/* return to windowed mode and move the window back to its original position */
		SDL_SetWindowFullscreen(win, 0);
		SDL_SetWindowPosition(win, prev_x, prev_y);

#ifdef OVR_OS_LINUX
		glcfg.OGL.Header.BackBufferSize = hmd->Resolution;

		distort_caps &= ~ovrDistortionCap_LinuxDevFullscreen;
		ovrHmd_ConfigureRendering(hmd, &glcfg.Config, distort_caps, hmd->DefaultEyeFov, eye_rdesc);
#endif
	}
}

void display()
{
	int i;
	ovrMatrix4f proj;
	ovrPosef pose[2];
	float rot_mat[16];

	/* the drawing starts with a call to ovrHmd_BeginFrame */
	ovrHmd_BeginFrame(hmd, 0);

	/* start drawing onto our texture render target */
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/* for each eye ... */
	for(i=0; i<2; i++) {
		ovrEyeType eye = hmd->EyeRenderOrder[i];

		/* -- viewport transformation --
		 * setup the viewport to draw in the left half of the framebuffer when we're
		 * rendering the left eye's view (0, 0, width/2, height), and in the right half
		 * of the framebuffer for the right eye's view (width/2, 0, width/2, height)
		 */
		glViewport(eye == ovrEye_Left ? 0 : fb_width / 2, 0, fb_width / 2, fb_height);

		/* -- projection transformation --
		 * we'll just have to use the projection matrix supplied by the oculus SDK for this eye
		 * note that libovr matrices are the transpose of what OpenGL expects, so we have to
		 * use glLoadTransposeMatrixf instead of glLoadMatrixf to load it.
		 */
		proj = ovrMatrix4f_Projection(hmd->DefaultEyeFov[eye], 0.5, 500.0, 1);
		glMatrixMode(GL_PROJECTION);
		glLoadTransposeMatrixf(proj.M[0]);

		/* -- view/camera transformation --
		 * we need to construct a view matrix by combining all the information provided by the oculus
		 * SDK, about the position and orientation of the user's head in the world.
		 */
		/* TODO: use ovrHmd_GetEyePoses out of the loop instead */
		pose[eye] = ovrHmd_GetHmdPosePerEye(hmd, eye);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();


		glTranslatef(eye_rdesc[eye].HmdToEyeViewOffset.x,
				eye_rdesc[eye].HmdToEyeViewOffset.y,
				eye_rdesc[eye].HmdToEyeViewOffset.z);
		/* retrieve the orientation quaternion and convert it to a rotation matrix */
		quat_to_matrix(&pose[eye].Orientation.x, rot_mat);
		glMultMatrixf(rot_mat);
		/* translate the view matrix with the positional tracking */
		glTranslatef(-pose[eye].Position.x, -pose[eye].Position.y, -pose[eye].Position.z);
		/* move the camera to the eye level of the user */
		glTranslatef(0, -ovrHmd_GetFloat(hmd, OVR_KEY_EYE_HEIGHT, 1.65), 0);
		glRotatef(ourAngle, 0, -1, 0);
		glTranslatef(xPos, yPos, zPos);

		/* finally draw the scene for this eye */
		draw_scene();
	}

	/* after drawing both eyes into the texture render target, revert to drawing directly to the
	 * display, and we call ovrHmd_EndFrame, to let the Oculus SDK draw both images properly
	 * compensated for lens distortion and chromatic abberation onto the HMD screen.
	 */
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	ovrHmd_EndFrame(hmd, pose, &fb_ovr_tex[0].Texture);

	/* workaround for the oculus sdk distortion renderer bug, which uses a shader
	 * program, and doesn't restore the original binding when it's done.
	 */
	glUseProgram(0);

	assert(glGetError() == GL_NO_ERROR);
}

void reshape(int x, int y)
{
	win_width = x;
	win_height = y;
}

void draw_cylinder() {
	glPushMatrix();
	glFrontFace(GL_CW);
	glRotatef(90.0,1.0,0.0,0.0);
	glTranslatef(0,0,-11);
	glEnable(GL_TEXTURE_2D);
	gluQuadricTexture(qobj, true);
	glBindTexture(GL_TEXTURE_2D, videoTexture);
	glColor3f(1,1,1);
	gluCylinder(qobj, 10.0, 10.0, 20.0, 20, 20);
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);
	glFrontFace(GL_CW);
	glPopMatrix();
}

void draw_scene(void)
{
	glMatrixMode(GL_MODELVIEW);

	draw_cylinder();
}

void draw_box(float xsz, float ysz, float zsz, float norm_sign)
{
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	glScalef(xsz * 0.5, ysz * 0.5, zsz * 0.5);

	if(norm_sign < 0.0) {
		glFrontFace(GL_CW);
	}

	glBegin(GL_QUADS);
	glNormal3f(0, 0, 1 * norm_sign);
	glTexCoord2f(0, 0); glVertex3f(-1, -1, 1);
	glTexCoord2f(1, 0); glVertex3f(1, -1, 1);
	glTexCoord2f(1, 1); glVertex3f(1, 1, 1);
	glTexCoord2f(0, 1); glVertex3f(-1, 1, 1);
	glNormal3f(1 * norm_sign, 0, 0);
	glTexCoord2f(0, 0); glVertex3f(1, -1, 1);
	glTexCoord2f(1, 0); glVertex3f(1, -1, -1);
	glTexCoord2f(1, 1); glVertex3f(1, 1, -1);
	glTexCoord2f(0, 1); glVertex3f(1, 1, 1);
	glNormal3f(0, 0, -1 * norm_sign);
	glTexCoord2f(0, 0); glVertex3f(1, -1, -1);
	glTexCoord2f(1, 0); glVertex3f(-1, -1, -1);
	glTexCoord2f(1, 1); glVertex3f(-1, 1, -1);
	glTexCoord2f(0, 1); glVertex3f(1, 1, -1);
	glNormal3f(-1 * norm_sign, 0, 0);
	glTexCoord2f(0, 0); glVertex3f(-1, -1, -1);
	glTexCoord2f(1, 0); glVertex3f(-1, -1, 1);
	glTexCoord2f(1, 1); glVertex3f(-1, 1, 1);
	glTexCoord2f(0, 1); glVertex3f(-1, 1, -1);
	glEnd();
	glBegin(GL_TRIANGLE_FAN);
	glNormal3f(0, 1 * norm_sign, 0);
	glTexCoord2f(0.5, 0.5); glVertex3f(0, 1, 0);
	glTexCoord2f(0, 0); glVertex3f(-1, 1, 1);
	glTexCoord2f(1, 0); glVertex3f(1, 1, 1);
	glTexCoord2f(1, 1); glVertex3f(1, 1, -1);
	glTexCoord2f(0, 1); glVertex3f(-1, 1, -1);
	glTexCoord2f(0, 0); glVertex3f(-1, 1, 1);
	glEnd();
	glBegin(GL_TRIANGLE_FAN);
	glNormal3f(0, -1 * norm_sign, 0);
	glTexCoord2f(0.5, 0.5); glVertex3f(0, -1, 0);
	glTexCoord2f(0, 0); glVertex3f(-1, -1, -1);
	glTexCoord2f(1, 0); glVertex3f(1, -1, -1);
	glTexCoord2f(1, 1); glVertex3f(1, -1, 1);
	glTexCoord2f(0, 1); glVertex3f(-1, -1, 1);
	glTexCoord2f(0, 0); glVertex3f(-1, -1, -1);
	glEnd();

	glFrontFace(GL_CCW);
	glPopMatrix();
}

/* update_rtarg creates (and/or resizes) the render target used to draw the two stero views */
void update_rtarg(int width, int height)
{
	if(!fbo) {
		/* if fbo does not exist, then nothing does... create every opengl object */
		glGenFramebuffers(1, &fbo);
		glGenTextures(1, &fb_tex);
		glGenRenderbuffers(1, &fb_depth);

		glBindTexture(GL_TEXTURE_2D, fb_tex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	/* calculate the next power of two in both dimensions and use that as a texture size */
	fb_tex_width = next_pow2(width);
	fb_tex_height = next_pow2(height);

	/* create and attach the texture that will be used as a color buffer */
	glBindTexture(GL_TEXTURE_2D, fb_tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, fb_tex_width, fb_tex_height, 0,
			GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fb_tex, 0);

	/* create and attach the renderbuffer that will serve as our z-buffer */
	glBindRenderbuffer(GL_RENDERBUFFER, fb_depth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, fb_tex_width, fb_tex_height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, fb_depth);

	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		fprintf(stderr, "incomplete framebuffer!\n");
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	printf("created render target: %dx%d (texture size: %dx%d)\n", width, height, fb_tex_width, fb_tex_height);
}

int handle_event(SDL_Event *ev)
{
	switch(ev->type) {
	case SDL_QUIT:
		return -1;

	case SDL_KEYDOWN:
	case SDL_KEYUP:
		if(key_event(ev->key.keysym.sym, ev->key.state == SDL_PRESSED) == -1) {
			return -1;
		}
		break;

	case SDL_WINDOWEVENT:
		if(ev->window.event == SDL_WINDOWEVENT_RESIZED) {
			reshape(ev->window.data1, ev->window.data2);
		}
		break;

	default:
		break;
	}

	return 0;
}

int key_event(int key, int state)
{
	if (state) {
		switch( key ){
			case SDLK_LEFT:
			case SDLK_a:
				xPos -= 1;
				break;
			case SDLK_RIGHT:
			case SDLK_d:
				xPos += 1;
				break;
			case SDLK_UP:
			case SDLK_w:
				zPos += 1;
				break;
			case SDLK_DOWN:
			case SDLK_s:
				zPos -= 1;
				break;
			case SDLK_q:
				ourAngle += 45;
				break;
			case SDLK_e:
				ourAngle -= 45;
				break;

			default:
				break;
		}
	}


	if(state) {
		ovrHSWDisplayState hsw;
		ovrHmd_GetHSWDisplayState(hmd, &hsw);
		if(hsw.Displayed) {
			ovrHmd_DismissHSWDisplay(hmd);
		}

		switch(key) {
		case 27:
			return -1;

		case ' ':
		case 'r':
			/* allow the user to recenter by pressing space */
			ovrHmd_RecenterPose(hmd);
			break;

		case 'f':
			/* press f to move the window to the HMD */
			toggle_hmd_fullscreen();
			break;

		case 'v':
			distort_caps ^= ovrDistortionCap_Vignette;
			printf("Vignette: %s\n", distort_caps & ovrDistortionCap_Vignette ? "on" : "off");
			ovrHmd_ConfigureRendering(hmd, &glcfg.Config, distort_caps, hmd->DefaultEyeFov, eye_rdesc);
			break;

		case 't':
			distort_caps ^= ovrDistortionCap_TimeWarp;
			printf("Time-warp: %s\n", distort_caps & ovrDistortionCap_TimeWarp ? "on" : "off");
			ovrHmd_ConfigureRendering(hmd, &glcfg.Config, distort_caps, hmd->DefaultEyeFov, eye_rdesc);
			break;

		case 'o':
			distort_caps ^= ovrDistortionCap_Overdrive;
			printf("OLED over-drive: %s\n", distort_caps & ovrDistortionCap_Overdrive ? "on" : "off");
			ovrHmd_ConfigureRendering(hmd, &glcfg.Config, distort_caps, hmd->DefaultEyeFov, eye_rdesc);
			break;

		case 'l':
			hmd_caps ^= ovrHmdCap_LowPersistence;
			printf("Low-persistence display: %s\n", hmd_caps & ovrHmdCap_LowPersistence ? "on" : "off");
			ovrHmd_SetEnabledCaps(hmd, hmd_caps);
			break;

		default:
			break;
		}
	}
	return 0;
}

unsigned int next_pow2(unsigned int x)
{
	x -= 1;
	x |= x >> 1;
	x |= x >> 2;
	x |= x >> 4;
	x |= x >> 8;
	x |= x >> 16;
	return x + 1;
}

/* convert a quaternion to a rotation matrix */
void quat_to_matrix(const float *quat, float *mat)
{
	mat[0] = 1.0 - 2.0 * quat[1] * quat[1] - 2.0 * quat[2] * quat[2];
	mat[4] = 2.0 * quat[0] * quat[1] + 2.0 * quat[3] * quat[2];
	mat[8] = 2.0 * quat[2] * quat[0] - 2.0 * quat[3] * quat[1];
	mat[12] = 0.0f;

	mat[1] = 2.0 * quat[0] * quat[1] - 2.0 * quat[3] * quat[2];
	mat[5] = 1.0 - 2.0 * quat[0]*quat[0] - 2.0 * quat[2]*quat[2];
	mat[9] = 2.0 * quat[1] * quat[2] + 2.0 * quat[3] * quat[0];
	mat[13] = 0.0f;

	mat[2] = 2.0 * quat[2] * quat[0] + 2.0 * quat[3] * quat[1];
	mat[6] = 2.0 * quat[1] * quat[2] - 2.0 * quat[3] * quat[0];
	mat[10] = 1.0 - 2.0 * quat[0]*quat[0] - 2.0 * quat[1]*quat[1];
	mat[14] = 0.0f;

	mat[3] = mat[7] = mat[11] = 0.0f;
	mat[15] = 1.0f;
}

/* generate a chessboard texture with tiles colored (r0, g0, b0) and (r1, g1, b1) */
unsigned int gen_chess_tex(float r0, float g0, float b0, float r1, float g1, float b1)
{
	int i, j;
	unsigned int tex;
	unsigned char img[8 * 8 * 3];
	unsigned char *pix = img;

	for(i=0; i<8; i++) {
		for(j=0; j<8; j++) {
			int black = (i & 1) == (j & 1);
			pix[0] = (black ? r0 : r1) * 255;
			pix[1] = (black ? g0 : g1) * 255;
			pix[2] = (black ? b0 : b1) * 255;
			pix += 3;
		}
	}

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 8, 8, 0, GL_RGB, GL_UNSIGNED_BYTE, img);

	return tex;
}
