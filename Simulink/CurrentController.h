#ifndef CURRENT_CONTROLLER_H
#define CURRENT_CONTROLLER_H

#define I_SAMPLE_RATE 50000
#define I_CONTROL_MAX_V 10.0
#define PWM_COUNTER_MAX 499

#define ICONTROL_KI 10
#define ICONTROL_KP 60

int PI_Iloop(double isamp, double isamp1, double isamp2, double isamp3, double iref, double bemf_estimation, double motor_vsupply);

#endif //CURRENT_CONTROLLER_H