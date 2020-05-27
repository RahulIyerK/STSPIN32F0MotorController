#ifndef __CURRENTCONTROLLER_H
#define __CURRENTCONTROLLER_H

#include <stdint.h>

#define HF_TIM_PERIOD (150-1)

// define KP by how current error (in ADC 12-bit representation) maps a current error to duty setpoint in range of 1-HF_ARR

#define I_CONTROL_KP 2 // map current error = 500mA to duty ~= 122/150

// KI affects how accumulated current error maps to increment in duty added to pterm-calculated duty
// with shift notation, we assign duty increment of 1 for every accumulated error crosses a multiple of 2^shift
#define I_CONTROL_KI_SHIFT 7 // = 1/128


/*
CURRENT SENSE: 
- current is sensed via ADC Channel 4
-current samples are 12-bit resolution stored in 32-bit integer
- therefore iref should also be a 12-bit maximum value stored in a 32-bit integer
   - as a further requirement, iref has an upper bound value because we want to current-limit the regulated current
- current error should be stored in 32-bit integer (difference of two 32-bit values)
*/

typedef struct ILoopParams_S
{
    int32_t ierr;
    int32_t ierr_acc;
    uint32_t iref;
} ILoopParams;


void CC_setCurrentReference(uint32_t ref);
uint32_t CC_runCurrentControlCycle();

void CC_processCurrent(uint32_t samp);
void CC_resetIntegral();

#endif //CURRENT_CONTROLLER_H
