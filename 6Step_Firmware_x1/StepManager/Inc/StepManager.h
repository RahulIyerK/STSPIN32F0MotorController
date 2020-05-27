#ifndef STEP_MANAGER_H
#define STEP_MANAGER_H
#include <stdint.h>
void SM_init(); //initialize Step Manager to Step 0

void SM_nextStep();

uint32_t SM_getBEMFChannel();

void SM_processBEMF(uint32_t bemf);
void SM_updateDuty(uint32_t val);

#endif //STEP_MANAGER_H

