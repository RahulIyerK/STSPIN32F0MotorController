#ifndef SPEED_CONTROLLER_H
#define SPEED_CONTROLLER_H

#define w_CONTROL_MIN_IREF 10
#define w_CONTROL_MAX_IREF 245

typedef struct wLoopParams_S
{
    int16_t werr_acc; // updated by ADC conversion interrupt
    uint16_t wref;    //updated by Step Manager (and speed control in closed-loop mode)
} wLoopParams;

#endif //SPEED_CONTROLLER_H
