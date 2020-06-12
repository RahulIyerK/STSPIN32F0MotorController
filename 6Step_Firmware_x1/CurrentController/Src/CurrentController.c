#include "CurrentController.h"
#include <math.h>
#include "fixptmath.h"

ILoopParams iloopParams;
extern volatile uint8_t force_zero_duty;

void CC_processCurrent(uint32_t samp)
{
    iloopParams.ierr = (int32_t)iloopParams.iref - (int32_t)samp;
    iloopParams.ierr_acc += iloopParams.ierr;
}

void CC_setCurrentReference(uint32_t ref)
{
    iloopParams.iref = ref;
}

uint16_t CC_runCurrentControlCycle()
{
	if (force_zero_duty)
	{
		return 0;
	}

    int32_t pterm = MUL32_Q0_UFIX_SFIX(I_CONTROL_KP, iloopParams.ierr);
    int32_t pwm_compare_val = ADD32_Q0_SFIX_SFIX((iloopParams.ierr_acc >> I_CONTROL_KI_SHIFT), pterm);

    //saturation
    if (pwm_compare_val < I_CONTROL_MIN_DUTY)
    {
        return I_CONTROL_MIN_DUTY;
    }
    
    if (pwm_compare_val > I_CONTROL_MAX_DUTY)
    {
        return I_CONTROL_MAX_DUTY;
    }

    return (uint16_t)pwm_compare_val;
}

void CC_resetIntegral()
{
    iloopParams.ierr_acc = 0;
}
