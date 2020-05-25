#ifndef STEP_MANAGER_H
#define STEP_MANAGER_H
#include <stdint.h>
void SM_init(); //initialize Step Manager to Step 0

void SM_nextStep();

void SM_sampleBEMF();
void SM_sampleCurrent();

void SM_setSwitchingDuty(uint16_t duty);

#endif //STEP_MANAGER_H

