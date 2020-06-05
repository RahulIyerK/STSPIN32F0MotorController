#include "SpeedController.h"
#include <stdint.h>

wLoopParams wloopParams;

void wC_setSpeedReference(uint16_t ref)
{
    wloopParams.wref = ref;
}

uint16_t wC_runSpeedControlCycle()
{
    int32_t pterm = MUL32_Q0_UFIX_SFIX(I_CONTROL_KP, iloopParams.ierr);
    int32_t pwm_compare_val = ADD32_Q0_SFIX_SFIX((iloopParams.ierr_acc >> I_CONTROL_KI_SHIFT), pterm);

    //saturation
    if (pwm_compare_val < I_CONTROL_MIN_PERIOD)
    {
        return I_CONTROL_MIN_PERIOD;
    }
    
    if (pwm_compare_val > I_CONTROL_MAX_PERIOD)
    {
        return I_CONTROL_MAX_PERIOD;
    }

    return (uint16_t)pwm_compare_val;
}

void wC_resetIntegral()
{
    wloopParams.ierr_acc = 0;
}



