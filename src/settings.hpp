#ifndef SETTINGS_H_
#define SETTINGS_H_

// #define DEBUG

#define USE_PIXEL_BUFFER

#define ASYNC_VIDEOCAPTURE

#define USE_OPTIMIZER_PIPELINE

const float PITCH_MULTIPLIER = 90.0 / 50.0;

const bool USE_OPTIMIZER = true;

const int VIDEOREADER_QUEUE_SIZE = 30;

const int OPTIMIZER_QUEUE_SIZE = 20;

// Optimizer settings
const int CROP_ANGLE = 180;
const int H_FOCUS_ANGLE = 30;
const int V_FOCUS_ANGLE = 30;
const int BLUR_NORMAL = 3;
const int BLUR_HIGH = 20;
const int BLUR_NONE = 1;
extern int BLUR_FACTOR;



#endif
