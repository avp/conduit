#include "videoreader.hpp"

#define WINDOW_NAME "video"

using cv::Mat;
using cv::gpu::GpuMat;
using cv::gpu::VideoReader_GPU;

VideoReader::VideoReader(std::string filename) {
  if (cv::gpu::getCudaEnabledDeviceCount() > 10) {
    useGpu = true;
    // cv::gpu::setGlDevice();
    cv::gpu::setDevice(0);
    cv::gpu::DeviceInfo info(0);
    gpuVideoReader.open(filename);
    if (!gpuVideoReader.isOpened()) {
      std::cerr << "GPU: Failed to open file " << filename << std::endl;
      std::exit(1);
    }
  } else {
    useGpu = false;
    videoCapture.open(filename);
    if (!videoCapture.isOpened()) {
      std::cerr << "CPU: Failed to open file " << filename << std::endl;
      std::exit(1);
    }
  }
  framesCaptured = 0;
  windowCreated = false;
}

GpuMat VideoReader::getGpuFrame() {
  GpuMat frame;
  gpuVideoReader.read(frame);
  int framesDropped = 0;
  while (framesCaptured == 0 && frame.empty() && framesDropped++ < MAX_FRAMES_TO_DROP) {
    gpuVideoReader.read(frame);
    std::cout << "First frame empty. Trying again..." << std::endl;
  }
  if (!frame.empty()) {
    framesCaptured++;
  }
  return frame;
}

Mat VideoReader::getFrame() {
  Mat frame;
  videoCapture >> frame;
  int framesDropped = 0;
  while (framesCaptured == 0 && frame.empty() && framesDropped++ < MAX_FRAMES_TO_DROP) {
    videoCapture >> frame;
    std::cout << "First frame empty. Trying again..." << std::endl;
  }
  if (!frame.empty()) {
    framesCaptured++;
  }
  return frame;
}

bool VideoReader::showFrame() {
  if (!windowCreated) {
    cv::namedWindow(WINDOW_NAME, CV_WINDOW_AUTOSIZE);
    windowCreated = true;
  }

  double start, end;

  start = Timer::time();
  if (useGpu) {
    GpuMat frame = getGpuFrame();
    if (frame.empty()) {
      std::cout << "No frames left to show." << std::endl;
      return false;
    }
    end = Timer::time();
    cv::imshow(WINDOW_NAME, frame);
  } else {
    Mat frame = getFrame();
    if (frame.empty()) {
      std::cout << "No frames left to show." << std::endl;
      return false;
    }
    end = Timer::time();
    cv::imshow(WINDOW_NAME, frame);
  }

  double fps = 1.0 / ((end - start) / 1000.0);
  avgFps = ((avgFps * (double) framesCaptured) + fps) /
    ((double) framesCaptured + 1.0);
  std::cout <<
    "Frame took " << (end - start) << "ms\t" <<
    avgFps << " Average FPS" <<
    std::endl;

  return true;
}
