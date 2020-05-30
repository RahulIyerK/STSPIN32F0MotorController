#ifndef STEP_MANAGER_DEFS_H
#define STEP_MANAGER_DEFS_H

#define LOWSIDE_PHASE_A (GPIO_PIN_13)
#define LOWSIDE_PHASE_B (GPIO_PIN_14)
#define LOWSIDE_PHASE_C (GPIO_PIN_15)

#define HIGHSIDE_PHASE_A (TIM_CHANNEL_1)
#define HIGHSIDE_PHASE_B (TIM_CHANNEL_2)
#define HIGHSIDE_PHASE_C (TIM_CHANNEL_3)

#define ADC_PHASE_A (ADC_CHANNEL_0)
#define ADC_PHASE_B (ADC_CHANNEL_1)
#define ADC_PHASE_C (ADC_CHANNEL_2)


#define BEMF_thresh 200

//ALIGNMENT STATE DEFINITION
#define ALIGNMENT_CURRENT_REF 41 // = 0.5A on a 50A scale mapped to 12-bit ADC max of 4095
#define ALIGNMENT_ARR (1000-1)
#define NUM_ALIGNMENT_PERIODS 1

//RAMP STATE DEFINITIONS
#define RAMP_CURRENT_REF 61
#define RAMP_TABLE_ENTRIES 99
//RUN STATE DEFINTIONS

#endif //STEP_MANAGER_DEFS_H
