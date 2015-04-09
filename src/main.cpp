#include <cstdlib>
#include <iostream>

#include "buildtest/buildtest.hpp"
#include "videoreader/videoreader.hpp"

static void usage() {
  std::cerr << "Please provide a runmode.\n" <<
    "Available runmodes:\n" <<
    "  buildtest\n" <<
    "  playvideo\n" <<
    std::endl;
  std::exit(1);
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    usage();
  }

  std::string runMode(argv[1]);

  if (runMode == "buildtest") {
    return BuildTest::runBuildTest();
  } else if (runMode == "playvideo") {
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
  } else {
    usage();
  }

  return 0;
}
