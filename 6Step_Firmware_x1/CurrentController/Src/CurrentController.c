#include "CurrentController.h"
#include <math.h>
#include "fixptmath.h"

ILoopParams iloopParams;

void CC_processCurrent(uint32_t samp)
{
    iloopParams.ierr = iloopParams.iref - samp;
    iloopParams.ierr_acc += iloopParams.ierr;
}

void CC_setCurrentReference(uint32_t ref)
{
    iloopParams.iref = ref;
}

uint32_t CC_runCurrentControlCycle()
{
    int32_t pterm = MUL32_Q0_UFIX_SFIX(I_CONTROL_KP, iloopParams.ierr);
    int32_t pwm_compare_val = ADD32_Q0_SFIX_SFIX((iloopParams.ierr_acc >> I_CONTROL_KI_SHIFT), pterm);

    //saturation
    if (pwm_compare_val < 0)
    {
        return 0;
    }
    
    if (pwm_compare_val > HF_TIM_PERIOD)
    {
        return HF_TIM_PERIOD;
    }

    return pwm_compare_val;
}

void CC_resetIntegral()
{
    iloopParams.ierr_acc = 0;
}