#ifndef TIMER_TIMER_H_
#define TIMER_TIMER_H_

#include <ctime>
#include <iostream>

#include "../settings.hpp"
#include "../CycleTimer.h"

class Timer {
  public:
    Timer() {}

    static double time() {
      return CycleTimer::currentSeconds() * 1000.0;
    }

    static double timeInSeconds() {
      return CycleTimer::currentSeconds();
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

static const int MAX_SAMPLES = 100;

class FramerateProfiler {
  public:
    FramerateProfiler() {
      for (int i = 0; i < MAX_SAMPLES; i++) {
        ticklist[i] = 0;
      }
    }

    void startFrame() {
      frameStart = Timer::timeInSeconds();
    }

    void endFrame() {
      profileFrame(Timer::timeInSeconds() - frameStart);
    }

    double getFramerate() {
      if (ticksum == 0)
        return 0;

      return samplesCollected/ticksum;
    }

    double getAverageTimeMillis() {
      if (samplesCollected == 0)
        return 0;

      return 1000 * ticksum/samplesCollected;
    }

  private:
    int tickindex = 0;
    double ticksum = 0;
    double ticklist[MAX_SAMPLES];
    int samplesCollected = 0;
    double frameStart = 0;

    /* average will ramp up until the buffer is full */
    /* returns average ticks per frame over the MAXSAMPPLES last frames */
    void profileFrame(double frameTime)
    {
        ticksum -= ticklist[tickindex];  /* subtract value falling off */
        ticksum += frameTime;              /* add new value */
        ticklist[tickindex] = frameTime;   /* save new value so it can be subtracted later */
        tickindex = (tickindex + 1) % MAX_SAMPLES;
        if (samplesCollected < MAX_SAMPLES)
          samplesCollected++;
    }
};

#endif
