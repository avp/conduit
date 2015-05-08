#ifndef VIDEOREADER_VIDEOREADER_H_
#define VIDEOREADER_VIDEOREADER_H_

#include <iostream>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

#include <opencv2/core/core.hpp>
#include <opencv2/core/opengl_interop.hpp>
#include <opencv2/gpu/gpu.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "../util/timer.hpp"
#include "../util/cudautil.hpp"
#include "../util/workqueue.h"

const int MAX_FRAMES_TO_DROP = 10;

class VideoReader {
  public:
    VideoReader(const std::string& filename);
    cv::Mat getFrame();
    cv::gpu::GpuMat getGpuFrame();
    bool showFrame();
    bool isFrameAvailable();

  private:
    void bufferFrames(const std::string& filename);

    cv::VideoCapture videoCapture;
    cv::gpu::VideoReader_GPU gpuVideoReader;
    bool useGpu;
    int framesCaptured;
    bool windowCreated;
    bool fullyBuffered;
    double avgFps;

    std::thread bufferThread;
    WorkQueue<cv::Mat> frameQueue;
    pthread_cond_t queueCond;
    pthread_mutex_t queueLock;
};

#endif
