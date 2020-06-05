#ifndef SPEED_CONTROLLER_H
#define SPEED_CONTROLLER_H

typedef struct wLoopParams_S
{
    int16_t werr_acc; // updated by ADC conversion interrupt
    uint16_t wref;    //updated by Step Manager (and speed control in closed-loop mode)
} wLoopParams;

#endif //SPEED_CONTROLLER_H