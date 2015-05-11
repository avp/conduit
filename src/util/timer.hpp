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

static const int MAX_SAMPLES = 50;

class RollingAverage {
  public:
    RollingAverage() {
      for (int i = 0; i < MAX_SAMPLES; i++) {
        ticklist[i] = 0;
      }
    }

    double getAverageReciprocal() {
      if (ticksum == 0)
        return 0;

      return samplesCollected/ticksum;
    }

    double getAverage() {
      if (samplesCollected == 0)
        return 0;

      return ticksum/samplesCollected;
    }

    /* average will ramp up until the buffer is full */
    /* returns average ticks per frame over the MAXSAMPPLES last frames */
    void addSample(double sample)
    {
        ticksum -= ticklist[tickindex];  /* subtract value falling off */
        ticksum += sample;              /* add new value */
        ticklist[tickindex] = sample;   /* save new value so it can be subtracted later */
        tickindex = (tickindex + 1) % MAX_SAMPLES;
        if (samplesCollected < MAX_SAMPLES)
          samplesCollected++;
    }

  private:
    int tickindex = 0;
    double ticksum = 0;
    double ticklist[MAX_SAMPLES];
    int samplesCollected = 0;

};

class FramerateProfiler {

  public:
    FramerateProfiler() {

    }

    void startFrame() {
      frameStart = Timer::timeInSeconds();
    }

    void endFrame() {
      myRollingAverage.addSample(Timer::timeInSeconds() - frameStart);
    }

    double getFramerate() {
      return myRollingAverage.getAverageReciprocal();
    }

    double getAverageTimeMillis() {
      return myRollingAverage.getAverage();
    }

  private:
    RollingAverage myRollingAverage;
    double frameStart = 0;

};

#endif
