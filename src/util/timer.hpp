#ifndef TIMER_TIMER_H_
#define TIMER_TIMER_H_

#include <ctime>
#include <iostream>

#include "../settings.hpp"

class Timer {
  public:
    Timer() {}

    static double time() {
      return std::clock() / (double) CLOCKS_PER_SEC * 1000.0;
    }

#ifdef DEBUG
    double startTime;
    void start() {
      startTime = time();
    }

    void stop(const char* timerName) {
      double total = time() - startTime;
      printf("%s = %.6f ms\n", timerName, total);
    }
#else
    void start() {}
    void stop(const char* timerName) {}
#endif

};

#endif
