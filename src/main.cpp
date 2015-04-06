#include <cstdlib>
#include <iostream>
#include <string>

#include "buildtest/buildtest.h"

static void usage() {
  std::cerr << "Please provide a runmode.\n" <<
    "Available runmodes:\n" <<
    "  buildtest" <<
    std::endl;
  std::exit(1);
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    usage();
  }

  std::string runmode(argv[1]);

  if (runmode == "buildtest") {
    return BuildTest::runBuildTest();
  }

  usage();

  return 0;
}
