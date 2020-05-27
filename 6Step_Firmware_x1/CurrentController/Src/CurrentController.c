#include "CurrentController.h"
#include <math.h>
#include "fixptmath.h"

volatile PI_params_S Iloop_params;


void PI_I_init()
{
    Iloop_params.ierr = 0;
    Iloop_params.integ_acc = 0;
}

int PI_Iloop()
{
    int32_t pterm = MUL32_Q0_UFIX_SFIX(I_CONTROL_KP, Iloop_params.ierr);
    int32_t va = ADD32_Q0_SFIX_SFIX((Iloop_params.integ_acc >> I_CONTROL_KI_SHIFT), pterm);

    if (va < 0)
    {
        return 0;
    }

    uint32_t pwm_compare_val = va * VA_DUTY_SCALE_SHIFT;
    
    if (pwm_compare_val > PWM_COUNTER_MAX)
    {
        return PWM_COUNTER_MAX;
    }

    return pwm_compare_val;
}
