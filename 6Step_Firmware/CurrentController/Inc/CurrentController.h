#ifndef __CURRENTCONTROLLER_H
#define __CURRENTCONTROLLER_H

#include <stdint.h>

#define PWM_COUNTER_MAX 499.f

#define I_CONTROL_KI 0.002f
#define I_CONTROL_KP 60.f

typedef struct PI_params_S
{
    uint32_t ierr;
    float integ_acc;
    float vsupply;
} PI_params_S;

int PI_Iloop();

#endif //CURRENT_CONTROLLER_H
