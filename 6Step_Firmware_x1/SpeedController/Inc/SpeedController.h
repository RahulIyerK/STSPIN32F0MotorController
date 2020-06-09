#ifndef SPEED_CONTROLLER_H
#define SPEED_CONTROLLER_H
#include <stdint.h>

#define W_CONTROL_COAST_IREF 10
#define W_CONTROL_MIN_DRIVE_IREF 20
#define W_CONTROL_MAX_DRIVE_IREF 80
#define W_CONTROL_KP 4

typedef struct wLoopParams_S
{
    int16_t werr_acc; // updated by ADC conversion interrupt
    uint16_t wref;    //updated by Step Manager (and speed control in closed-loop mode)
} wLoopParams;

void wC_setSpeedReference(uint16_t ref);
uint32_t wC_runSpeedControlCycle(uint16_t wsamp);
void wC_resetIntegral();

#endif //SPEED_CONTROLLER_H
