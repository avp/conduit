#ifndef OCULUS2_OCULUS2_H_
#define OCULUS2_OCULUS2_H_

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
