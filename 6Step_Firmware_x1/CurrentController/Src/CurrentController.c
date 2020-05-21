#include "CurrentController.h"
#include <math.h>

volatile PI_params_S Iloop_params = {0,0,0};

int PI_Iloop()
{
    // isamp: latest current sample
    // iref:  current control reference
    // bemf_estimation: estimation of back-emf voltage magnitude based on latest speed sample
    // motor_vsupply: DC supply across 3-phase H-bridge



    float va = I_CONTROL_KI * Iloop_params.integ_acc + I_CONTROL_KP * Iloop_params.ierr; //PI term summation + feedforward term from bemf estimation
    float duty = va/Iloop_params.vsupply;

    // saturation
    if (va > Iloop_params.vsupply)
    {
        return PWM_COUNTER_MAX;
    }
    else if (va < 0)
    {
        return 0;
    }

    //on-time duty


    //result pwm register compare value depends on how on-time is defined with respect to it

    int pwm_compare_val = (int)(duty * PWM_COUNTER_MAX);

    return pwm_compare_val;
//	return 0;
}
