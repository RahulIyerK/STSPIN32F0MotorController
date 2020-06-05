#include "SpeedController.h"
#include <stdint.h>

wLoopParams wloopParams;

void wC_setSpeedReference(uint16_t ref)
{
    wloopParams.wref = ref;
}

uint32_t wC_runSpeedControlCycle(uint16_t wsamp)
{
	int16_t werr =  (int16_t)wloopParams.wref - (int16_t)wsamp;
	wloopParams.werr_acc += werr;

	int32_t iref = werr * W_CONTROL_KP; //+ wloopParams.werr_acc * W_CONTROL_KI;

    //saturation
    if (iref < W_CONTROL_MIN_IREF)
    {
        return W_CONTROL_MIN_IREF;
    }

    if (iref > W_CONTROL_MAX_IREF)
    {
        return W_CONTROL_MAX_IREF;
    }

    return (uint32_t)iref;


}

void wC_resetIntegral()
{
    wloopParams.werr_acc = 0;
}



