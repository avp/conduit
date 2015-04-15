#include <cstdlib>
#include <iostream>

#include "buildtest/buildtest.hpp"
#include "videoreader/videoreader.hpp"
#include "renderer/renderer.hpp"

static void usage() {
  std::cerr << "Please provide a runmode.\n" <<
    "Available runmodes:\n" <<
    "  buildtest\n" <<
    "  playvideo\n" <<
    "  render\n" <<
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
    cv::waitKey(30);
  }
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
  // Renderer renderer(1024, 640);
  Renderer::displayStereoImage(image);
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
  } else if (runMode == "render") {
    return renderStereo(argc, argv);
  } else {
    usage();
  }

  return 0;
}
