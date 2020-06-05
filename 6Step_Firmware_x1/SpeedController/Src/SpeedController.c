#include "SpeedController.h"
#include <stdint.h>

wLoopParams wloopParams;

void wC_setSpeedReference(uint16_t ref)
{
    wloopParams.wref = ref;
}

uint16_t wC_runSpeedControlCycle()
{

}

void wC_resetIntegral()
{
    wloopParams.ierr_acc = 0;
}



