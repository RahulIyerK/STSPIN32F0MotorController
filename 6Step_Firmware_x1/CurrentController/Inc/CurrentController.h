#ifndef __CURRENTCONTROLLER_H
#define __CURRENTCONTROLLER_H

#include <stdint.h>

#define PWM_COUNTER_MAX (100-1)

#define I_CONTROL_KP 60
#define I_CONTROL_KI_SHIFT 5 // Ki = 0.03125
#define VA_DUTY_SCALE_SHIFT 24 // gain of 6e-8


/*
CURRENT SENSE: 
- current is sensed via ADC Channel 4
-current samples are 12-bit resolution stored in 32-bit integer
- therefore iref should also be a 12-bit maximum value stored in a 32-bit integer
   - as a further requirement, iref has an upper bound value because we want to current-limit the regulated current
- current error should be stored in 32-bit integer (difference of two 32-bit values)
*/


typedef struct PI_params_S
{
    int32_t ierr;
    int32_t integ_acc;
} PI_params_S;

void PI_I_init();
int PI_Iloop();

#endif //CURRENT_CONTROLLER_H
