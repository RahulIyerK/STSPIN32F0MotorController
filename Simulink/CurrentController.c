#include "CurrentController.h"
#include <math.h>
double Iloop_error_acc = 0;
int PI_Iloop(double isamp, double isamp1, double isamp2, double isamp3, double iref, double bemf_estimation, double motor_vsupply)
{
    // isamp: latest current sample
    // iref:  current control reference
    // bemf_estimation: estimation of back-emf voltage magnitude based on latest speed sample
    // motor_vsupply: DC supply across 3-phase H-bridge

    double error = iref - isamp;
    double error1 = iref - isamp1;
    double error2 = iref - isamp2;
    double error3 = iref - isamp3;

    Iloop_error_acc += (error + error1 + error2 + error3)/I_SAMPLE_RATE;

    double iterm = ICONTROL_KI * Iloop_error_acc;
    double pterm = ICONTROL_KP * error;
    double va = iterm + pterm; //+ bemf_estimation; //PI term summation + feedforward term from bemf estimation

    // saturation
    if (va > I_CONTROL_MAX_V)
    {
        va = I_CONTROL_MAX_V;
    }
    else if (va < 0)
    {
        va  = 0; 
    }

    //on-time duty

    double duty = va /motor_vsupply;

    //result pwm register compare value depends on how on-time is defined with respect to it

    int pwm_compare_val = PWM_COUNTER_MAX - ((int)(round(duty * PWM_COUNTER_MAX))); 

    return pwm_compare_val;

}
