#include "StepManager.h"
#include "StepManager_defs.h"
#include "CurrentController.h"
#include "stm32f0xx_hal.h"
#include <stdint.h>

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;
extern ADC_HandleTypeDef hadc;

uint32_t currentBemfAdcChannel;

static inline void ADC_Channel(uint32_t adc_ch)
{

  hadc.Instance->CR |= ADC_CR_ADSTP;
  while(hadc.Instance->CR & ADC_CR_ADSTP);
  /* Regular sequence configuration */
  /* Set the channel selection register from the selected channel */
  hadc.Instance->CHSELR = ADC_CHSELR_CHANNEL(adc_ch);
  hadc.Instance->CR |= ADC_CR_ADSTART;

}


typedef enum CONTROL_STATE_E
{
    ALIGNMENT,  // rotor alignment
    RAMP,       // startup speed ramp (6-step, speed increase)
    RUN         // autocommutation (closed loop speed control)
} CONTROL_STATE;

CONTROL_STATE controlState;

// STEPS 0 through 5:
//   HIGH  |   LOW   |  OPEN
// -------------------------
// PHASE_C | PHASE_B | PHASE_A
// PHASE_A | PHASE_B | PHASE_C
// PHASE_A | PHASE_C | PHASE_B
// PHASE_B | PHASE_C | PHASE_A
// PHASE_B | PHASE_A | PHASE_C
// PHASE_C | PHASE_A | PHASE_B  

int curr_step = 0;
uint8_t alignment_index = 0;
uint8_t ramp_index = 0;


uint16_t ramp_table [RAMP_TABLE_ENTRIES] = 
{
1000,
1000,
1000,
1000,
1000,
1000,
1000,
500,
500,
500,
500,
500,
500,
500,
500,
300,
300,
300,
300,
300,
250,
250,
250,
250,
250,
200,
200,
200,
200,
200,
180,
180,
180,
180,
180,
155,
155,
155,
155,
155,
140,
140,
140,
140,
140,
130,
130,
130,
130,
130,
120,
120,
120,
120,
120
};

uint16_t SM_fetchRampARR(uint8_t index)
{
    return ramp_table[index];
}


void configStep()
{
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, 0);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, 0);
    // TODO: probably don't have to be this aggressive about turning everything off before changing steps

    CC_resetIntegral(); // TODO: how often should integral be reset?

    switch (controlState)
    {
        case ALIGNMENT:
        {
            // align to intermediate position before step 0
            HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_B, GPIO_PIN_SET);   // set   PHASE_B GPIO
            HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_C, GPIO_PIN_RESET); // reset PHASE_C GPIO
            HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_A, GPIO_PIN_SET);   // set PHASE_A GPIO
            CC_setCurrentReference(ALIGNMENT_CURRENT_REF);

            alignment_index++;
            if (alignment_index == NUM_ALIGNMENT_PERIODS)
            {
                controlState = RAMP;
                ramp_index = 0; // make sure ramp starts on index 0
                alignment_index = 0; //reset for future alignments (?)
            }
        }
        break;
        case RAMP:
        {
            CC_setCurrentReference(RAMP_CURRENT_REF);
            if (ramp_index < RAMP_TABLE_ENTRIES)
            {
                uint16_t arr = SM_fetchRampARR(ramp_index); //set next step duration
                __HAL_TIM_SET_AUTORELOAD(&htim2, arr);
                ramp_index++;
            }

            //TODO: startup zero-cross detection validation
            
        }
        break;
        case RUN:
        {
            //TODO: set current reference based on speed control cycle output

            switch (curr_step)
            {
                case 0:
                {
                    HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_B, GPIO_PIN_SET);   // set   PHASE_B GPIO
                    HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_C, GPIO_PIN_RESET); // reset PHASE_C GPIO
                    HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_A, GPIO_PIN_RESET); // reset PHASE_A GPIO
                }
                break;
                case 1:
                {
                    HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_B, GPIO_PIN_SET);   // set   PHASE_B
                    HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_A, GPIO_PIN_RESET); // reset PHASE_A
                    HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_C, GPIO_PIN_RESET); // reset PHASE_C
                }
                break;
                case 2:
                {
                    HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_C, GPIO_PIN_SET);   // set   PHASE_C
                    HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_A, GPIO_PIN_RESET); // reset PHASE_A
                    HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_B, GPIO_PIN_RESET); // reset PHASE_B
                }
                break;
                case 3:
                {
                    HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_C, GPIO_PIN_SET);   // set   PHASE_C
                    HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_B, GPIO_PIN_RESET); // reset PHASE_B
                    HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_A, GPIO_PIN_RESET); // reset PHASE_A
                }
                break;
                case 4:
                {
                    HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_A, GPIO_PIN_SET);   // set   PHASE_A
                    HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_B, GPIO_PIN_RESET); // reset PHASE_B
                    HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_C, GPIO_PIN_RESET); // reset PHASE_C
                }
                break;
                case 5:
                {
                    HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_A, GPIO_PIN_SET);   // set   PHASE_A
                    HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_C, GPIO_PIN_RESET); // reset PHASE_C
                    HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_B, GPIO_PIN_RESET); // reset PHASE_B  
                }
                break;

            }
        }
        break;
    }
    

}




void SM_init()
{

    currentBemfAdcChannel = 0;//zero out the current ADC channel

    __HAL_TIM_SET_COMPARE(&htim1, HIGHSIDE_PHASE_A, 0);     // set PWM duty to 0
    __HAL_TIM_SET_COMPARE(&htim1, HIGHSIDE_PHASE_B, 0);     // set PWM duty to 0
    __HAL_TIM_SET_COMPARE(&htim1, HIGHSIDE_PHASE_C, 0);     // set PWM duty to 0

    HAL_TIM_PWM_Start(&htim1,TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim1,TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim1,TIM_CHANNEL_3);

    HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_A, GPIO_PIN_RESET); // open PHASE_A LS FET
    HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_B, GPIO_PIN_RESET); // open PHASE_B LS FET
    HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_C, GPIO_PIN_RESET); // open PHASE_C LS FET

    curr_step = 0;
    alignment_index = 0;
    ramp_index = 0;
    controlState = ALIGNMENT; //start up in ALIGNMENT
    configStep();

}

void SM_nextStep()
{
    if (controlState == RAMP || controlState == RUN)
    {
        curr_step++;

        if (curr_step >= 6) //circular
        {
            curr_step = 0;
        }
    }

    configStep();

    if (controlState == RAMP || controlState == RUN)
    {
        //synchronize the ADC read with the step switching
        switch (curr_step)
        {
            case 0:
                currentBemfAdcChannel = ADC_PHASE_A;
            break;
            case 1:
                currentBemfAdcChannel = ADC_PHASE_C;
            break;
            case 2:
                currentBemfAdcChannel = ADC_PHASE_B;
            break;
            case 3:
                currentBemfAdcChannel = ADC_PHASE_A;
            break;
            case 4:
                currentBemfAdcChannel = ADC_PHASE_C;
            break;
            case 5:
                currentBemfAdcChannel = ADC_PHASE_B;
            break;
        }

        if(__HAL_TIM_IS_TIM_COUNTING_DOWN(&htim1))
        {
            ADC_Channel(currentBemfAdcChannel);
        }
    }
}

uint32_t SM_getBEMFChannel()
{
    return currentBemfAdcChannel;
}

void SM_processBEMF(uint32_t bemf)
{
    if (controlState == RUN)
    {
        /*
            set ARR
        */
          // if(bemf < BEMF_thresh){
//		__HAL_TIM_SET_AUTORELOAD(&htim2, __HAL_TIM_GET_COUNTER(&htim2) << 1);
	    //}
    }
}

void SM_updateDuty(uint32_t duty)
{
    switch (curr_step)
    {
        case 0:
            __HAL_TIM_SET_COMPARE(&htim1, HIGHSIDE_PHASE_C, duty); 
        break;
        case 1:
            __HAL_TIM_SET_COMPARE(&htim1, HIGHSIDE_PHASE_A, duty); 
        break;
        case 2:
            __HAL_TIM_SET_COMPARE(&htim1, HIGHSIDE_PHASE_A, duty); 
        break;
        case 3:
            __HAL_TIM_SET_COMPARE(&htim1, HIGHSIDE_PHASE_B, duty); 
        break;
        case 4:
            __HAL_TIM_SET_COMPARE(&htim1, HIGHSIDE_PHASE_B, duty); 
        break;
        case 5:
            __HAL_TIM_SET_COMPARE(&htim1, HIGHSIDE_PHASE_C, duty); 
        break;
    }
}
