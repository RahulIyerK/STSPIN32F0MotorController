#include "SpeedController.h"
#include <stdint.h>

wLoopParams wloopParams;

void wC_setSpeedReference(uint16_t ref)
{
    wloopParams.wref = ref;
}

uint32_t wC_runSpeedControlCycle(uint16_t wsamp)
{
	int16_t werr = (int16_t)wsamp - (int16_t)wloopParams.wref ;
	wloopParams.werr_acc += werr;

	int32_t iref = werr * W_CONTROL_KP; //+ wloopParams.werr_acc * W_CONTROL_KI;

	iref += W_CONTROL_MIN_DRIVE_IREF;
    //saturation
    if (iref < W_CONTROL_COAST_IREF)
    {
        return W_CONTROL_COAST_IREF;
    }

    if (iref > W_CONTROL_MAX_DRIVE_IREF)
    {
        return W_CONTROL_MAX_DRIVE_IREF;
    }

    return (uint32_t)iref;


}

void wC_resetIntegral()
{
    wloopParams.werr_acc = 0;
}



