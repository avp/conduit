#ifndef TIMER_TIMER_H_
#define TIMER_TIMER_H_

#include <ctime>

class Timer {
  public:
    static double time() {
      return std::clock() / (double) CLOCKS_PER_SEC * 1000.0;
    }
};

#endif
