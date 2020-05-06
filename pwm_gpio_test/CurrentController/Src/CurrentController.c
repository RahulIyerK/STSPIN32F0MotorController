#include "CurrentController.h"
#include <math.h>
struct PI_params_S Iloop_params = {12579, 62.8947, 0.0159, 0, 0, 0};
int PI_Iloop(double isamp, double iref, double bemf_estimation, double motor_vsupply)
{
    // isamp: latest current sample
    // iref:  current control reference
    // bemf_estimation: estimation of back-emf voltage magnitude based on latest speed sample
    // motor_vsupply: DC supply across 3-phase H-bridge

    double error = iref - isamp;
    Iloop_params.integ_acc += (error - Iloop_params.Ka * (Iloop_params.va - Iloop_params.va_limited))/I_SAMPLE_RATE;

    double iterm = Iloop_params.Ki * Iloop_params.integ_acc;
    double pterm = Iloop_params.Kp * error;
    Iloop_params.va = iterm + pterm + bemf_estimation; //PI term summation + feedforward term from bemf estimation

    // saturation
    if (Iloop_params.va > I_CONTROL_MAX_V)
    {
        Iloop_params.va_limited = I_CONTROL_MAX_V;
    }
    else 
    {
        Iloop_params.va_limited = Iloop_params.va;
    }

    //on-time duty

    double duty = Iloop_params.va_limited/motor_vsupply;

    //result pwm register compare value depends on how on-time is defined with respect to it

    int pwm_compare_val = PWM_COUNTER_MAX - ((int)(round(duty * PWM_COUNTER_MAX))); 

    return pwm_compare_val;
}
