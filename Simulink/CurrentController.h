#ifndef CURRENT_CONTROLLER_H
#define CURRENT_CONTROLLER_H

#define I_SAMPLE_RATE 50000
#define I_CONTROL_MAX_V 10.0
#define PWM_COUNTER_MAX 499

struct PI_params_S
{
    double Ki;
    double Kp;
    double Ka;
    double integ_acc;
    double va;
    double va_limited;
};

struct PI_params_S Iloop_params;

uint16_t PI_Iloop(double isamp, double iref, double bemf_estimation, double motor_vsupply);

#endif //CURRENT_CONTROLLER_H