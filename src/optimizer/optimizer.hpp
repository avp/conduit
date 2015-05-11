#ifndef OPTIMIZER_OPTIMIZER_H_
#define OPTIMIZER_OPTIMIZER_H_

#include <mutex>
#include <queue>
#include <thread>
#include <vector>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "../util/imageutil.hpp"
#include "../util/timer.hpp"
#include "../contracts.h"
#include "../settings.hpp"
#include "../videoreader/videoreader.hpp"

class FrameData {
  public:
    FrameData(const cv::Mat& image, double timestamp);

    cv::Mat image;
    double timestamp;
};

class OptimizedImage {
  friend class Optimizer;

  public:
    size_t size() const;

  private:
    OptimizedImage(const cv::Mat& focusedTop,
        const cv::Mat& focusedBot,
        const cv::Mat& blurred,
        int focusRow, int focusCol,
        cv::Size croppedSize, cv::Size fullSize, int leftBuffer);

    cv::Mat focusedTop;
    cv::Mat focusedBot;
    cv::Mat blurred;
    int focusRow;
    int focusCol;
    cv::Size croppedSize;
    cv::Size fullSize;
    int leftBuffer;
};

class Optimizer {
  public:
    static OptimizedImage optimizeImage(const cv::Mat& image,
        int angle, int vAngle);
    static cv::Mat extractImage(const OptimizedImage& image);
    static cv::Mat processImage(const cv::Mat& image,
        int angle, int vAngle);
};

class OptimizerPipeline {
  public:
    OptimizerPipeline(VideoReader* vr);
    FrameData getFrame();
    bool isFrameAvailable();
    volatile int hAngle = 0;
    volatile int vAngle = 90;
    volatile double lastUpdated = 0;
    std::mutex hmdDataMutex;
    int getNumFramesAvailable();

  private:
    void bufferFrames(VideoReader* vr);
    WorkQueue<FrameData> frameQueue;
    pthread_cond_t queueCond;
    pthread_mutex_t queueLock;
    std::thread bufferThread;
    bool fullyBuffered;
    bool frameAvailable;
};

#endif
