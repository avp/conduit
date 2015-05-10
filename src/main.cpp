#include <cstdlib>
#include <iostream>

#include "buildtest/buildtest.hpp"
#include "oculus2/oculus2.hpp"
#include "optimizer/optimizer.hpp"
#include "renderer/renderer.hpp"
#include "rendertest/rendertest.hpp"
#include "util/cylinderwarp.hpp"
#include "util/imageutil.hpp"
#include "util/timer.hpp"
#include "videoreader/videoreader.hpp"

static void usage() {
  std::cerr << "Please provide a runmode.\n" <<
    "Available runmodes:\n" <<
    "  buildtest\n" <<
    "  playvideo\n" <<
    "  cylinderwarp\n" <<
    "  render\n" <<
    "  rendertest\n" <<
    "  oculus2\n" <<
    "  optimize\n" <<
    std::endl;
  std::exit(1);
}

static int playVideo(int argc, char* argv[]) {
  if (argc < 3) {
    std::cerr << "Usage: "
      << argv[0]
      << " playvideo filename"
      << std::endl;
    return 1;
  }
  std::string filename = argv[2];
  VideoReader videoReader(filename);
  while (videoReader.showFrame()) {
    cv::waitKey(1);
  }
  return 0;
}

static int cylinderWarp(int argc, char* argv[]) {
  if (argc < 3) {
    std::cerr << "Usage: "
      << argv[0]
      << " cylinderwarp filename"
      << std::endl;
    return 1;
  }
  std::string filename = argv[2];
  VideoReader videoReader(filename);
  cv::Mat image = videoReader.getFrame();
  cv::Mat left = cv::Mat(image, cv::Range(0, image.rows / 2));
  cv::Mat warpedImage = CylinderWarp::cylinderWarp(left);
  std::string windowName = "Warped image";
  cv::namedWindow(windowName, CV_WINDOW_NORMAL);
  cv::imshow(windowName, warpedImage);
  cv::waitKey(0);
  return 0;
}

static int renderStereo(int argc, char* argv[]) {
  if (argc < 3) {
    std::cerr << "Usage: "
      << argv[0]
      << " render filename"
      << std::endl;
    return 1;
  }
  std::string filename = argv[2];
  VideoReader videoReader(filename);
  cv::Mat image = videoReader.getFrame();
  glutInit(&argc, argv);
  Renderer renderer(1024, 640);
  renderer.displayStereoImage(image);
  return 0;
}

static int renderTest(int argc, char* argv[]) {
  if (argc < 3) {
    std::cerr << "Usage: "
      << argv[0]
      << " rendertest filename"
      << std::endl;
    return 1;
  }
  std::string filename = argv[2];
  VideoReader videoReader(filename);
  cv::Mat image = videoReader.getFrame();
  cv::Mat left = cv::Mat(image, cv::Range(0, image.rows / 2));

  return RenderTest::renderTest(argc, argv, left);
}

static int oculus2(int argc, char* argv[]) {
  return Oculus2::run(argc, argv);
}

static int optimize(int argc, char* argv[]) {
  if (argc < 3) {
    std::cerr << "Usage: "
      << argv[0]
      << " optimize filename"
      << std::endl;
    return 1;
  }
  std::string filename = argv[2];
  VideoReader videoReader(filename);

  cv::Mat left, extractedLeft;
  Timer timer;

  for (int i = 0; i < 1; i++) {
    std::cout << "\nFrame " << i << std::endl;

    timer.start();
    cv::Mat image = cv::Mat(videoReader.getFrame());
    timer.stop("Get frame");

    if (image.empty()) {
      std::cout << "No frames left to show." << std::endl;
      return 1;
    }

    left = cv::Mat(image, cv::Range(0, image.rows / 2));

    std::cout << "Optimizing image..." << std::endl;
    timer.start();
    OptimizedImage optLeft = Optimizer::optimizeImage(left, 180, 90);
    timer.stop("Time");

    size_t beforeSize = ImageUtil::imageSize(left);
    size_t afterSize = optLeft.size();
    double ratio = ((double) afterSize) / ((double) beforeSize) * 100.0;
    std::cout << "Optimized: " << beforeSize << " -> " << afterSize << std::endl;
    std::cout << "Ratio:     " << ratio << "%" << std::endl;

    std::cout << "Extracting image..." << std::endl;
    timer.start();
    extractedLeft = Optimizer::extractImage(optLeft);
    timer.stop("Time");
  }

  // cv::namedWindow("Before", CV_WINDOW_NORMAL);
  // cv::imshow("Before", left);

  // cv::namedWindow("After", CV_WINDOW_NORMAL);
  // cv::imshow("After", extractedLeft);

  cv::waitKey(0);
  return 0;
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    usage();
  }

  std::string runMode(argv[1]);

  if (runMode == "buildtest") {
    return BuildTest::runBuildTest();
  } else if (runMode == "playvideo") {
    return playVideo(argc, argv);
  } else if (runMode == "cylinderwarp") {
    return cylinderWarp(argc, argv);
  } else if (runMode == "render") {
    return renderStereo(argc, argv);
  } else if (runMode == "rendertest") {
    return renderTest(argc, argv);
  } else if (runMode == "oculus2") {
    return oculus2(argc, argv);
  } else if (runMode == "optimize") {
    return optimize(argc, argv);
  } else {
    usage();
  }

  return 0;
}
