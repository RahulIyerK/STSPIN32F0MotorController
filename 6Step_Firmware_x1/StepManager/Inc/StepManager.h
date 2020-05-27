#ifndef STEP_MANAGER_H
#define STEP_MANAGER_H
#include <stdint.h>
void SM_init(); //initialize Step Manager to Step 0

void SM_nextStep();

uint32_t SM_getBEMFChannel();

void SM_processBEMF(uint32_t bemf);
void SM_updateDuty(uint32_t val);

extern inline void ADC_Channel(uint32_t adc_ch); //TODO: this is a hack...

#endif //STEP_MANAGER_H

