#ifndef SPEED_CONTROLLER_H
#define SPEED_CONTROLLER_H
#include <stdint.h>

#define W_CONTROL_MIN_IREF 10
#define W_CONTROL_MAX_IREF 245
#define W_CONTROL_KP 35

typedef struct wLoopParams_S
{
    int16_t werr_acc; // updated by ADC conversion interrupt
    uint16_t wref;    //updated by Step Manager (and speed control in closed-loop mode)
} wLoopParams;

#endif //SPEED_CONTROLLER_H
