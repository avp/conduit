#ifndef OCULUS2_OCULUS2_H_
#define OCULUS2_OCULUS2_H_

#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <thread>

#ifdef WIN32
#include <SDL.h>
#else
#include <SDL2/SDL.h>
#endif
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
#include <Extras/OVR_Math.h>

#include "../contracts.h"
#include "../videoreader/videoreader.hpp"

class TextureData {
	public:
		TextureData();
		void init();
		void load(const cv::Mat& image);

		GLuint name = 0;
		GLuint pbo = 0;
		int width = 0;
		int height = 0;
		size_t size = 0;
		bool initialized = false;
		bool loaded = false;
};

class Oculus2 {
  public:
  	static int run(int argc, char **argv);
};

#endif
