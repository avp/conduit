#ifndef VIDEOREADER_VIDEOREADER_H_
#define VIDEOREADER_VIDEOREADER_H_

#include <iostream>
#include <string>

#include <opencv2/core/core.hpp>
#include <opencv2/core/opengl_interop.hpp>
#include <opencv2/gpu/gpu.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "../timer/timer.hpp"
#include "../util/cudautil.hpp"

const int MAX_FRAMES_TO_DROP = 10;

class VideoReader {
  public:
    VideoReader(std::string filename);
    cv::Mat getFrame();
    cv::gpu::GpuMat getGpuFrame();
    bool showFrame();

  private:
    cv::VideoCapture videoCapture;
    cv::gpu::VideoReader_GPU gpuVideoReader;
    bool useGpu;
    int framesCaptured;
    bool windowCreated;
    double avgFps;
};

#endif
