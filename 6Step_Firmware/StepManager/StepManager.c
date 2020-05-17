#include "StepManager.h"
#include "stm32f0xx_hal_tim.h"
#include "stm32f0xx_hal_gpio.h"
#include "stm32f0xx_hal_adc.h"
#include <stdint.h>

extern TIM_HandleTypeDef htim1;
extern ADC_HandleTypeDef hadc;


#define LOWSIDE_PHASE_A (GPIO_PIN_13)
#define LOWSIDE_PHASE_B (GPIO_PIN_14)
#define LOWSIDE_PHASE_C (GPIO_PIN_15)

#define HIGHSIDE_PHASE_A (TIM_CHANNEL_1)
#define HIGHSIDE_PHASE_B (TIM_CHANNEL_2)
#define HIGHSIDE_PHASE_C (TIM_CHANNEL_3)

#define ADC_PHASE_A (ADC_CHANNEL_0)
#define ADC_PHASE_B (ADC_CHANNEL_1)
#define ADC_PHASE_C (ADC_CHANNEL_2)

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

void configStep()
{
    
    // TODO: probably don't have to be this aggressive about turning everything off before changing steps

    HAL_TIM_PWM_Stop(&htim1,TIM_CHANNEL_1);
    HAL_TIM_PWM_Stop(&htim1,TIM_CHANNEL_2);
    HAL_TIM_PWM_Stop(&htim1,TIM_CHANNEL_3);

    switch (curr_step)
    {
        case 0:
            HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_B, GPIO_PIN_SET);   // set   PHASE_B GPIO
            HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_C, GPIO_PIN_RESET); // reset PHASE_C GPIO
            HAL_TIM_PWM_Start(&htim1, HIGHSIDE_PHASE_C);               // start PHASE_C PWM
        break;
        case 1:
            HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_B, GPIO_PIN_SET);   // set   PHASE_B
            HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_A, GPIO_PIN_RESET); // reset PHASE_A
            HAL_TIM_PWM_Start(&htim1, HIGHSIDE_PHASE_A);               // start PHASE_A PWM
        break;
        case 2:
            HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_C, GPIO_PIN_SET);   // set   PHASE_C
            HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_A, GPIO_PIN_RESET); // reset PHASE_A
            HAL_TIM_PWM_Start(&htim1, HIGHSIDE_PHASE_A);               // start PHASE_A PWM  
        break;
        case 3:
            HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_C, GPIO_PIN_SET);   // set   PHASE_C
            HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_B, GPIO_PIN_RESET); // reset PHASE_B
            HAL_TIM_PWM_Start(&htim1, HIGHSIDE_PHASE_B);               // start PHASE_B PWM       
        break;
        case 4:
            HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_A, GPIO_PIN_SET);   // set   PHASE_A
            HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_B, GPIO_PIN_RESET); // reset PHASE_B
            HAL_TIM_PWM_Start(&htim1, HIGHSIDE_PHASE_B);               // start PHASE_B PWM 
        break;
        case 5:
            HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_A, GPIO_PIN_SET);   // set   PHASE_A
            HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_C, GPIO_PIN_RESET); // reset PHASE_C
            HAL_TIM_PWM_Start(&htim1, HIGHSIDE_PHASE_C);               // start PHASE_C PWM       
        break;

    }
}

void SM_init()
{
    curr_step = 0;
    configStep();
}

void SM_nextStep()
{
    curr_step++;

    if (curr_step >= 6) //circular
    {
        curr_step = 0;
    }
    
    configStep();
}

uint32_t SM_sampleBEMF()
{
    switch (curr_step)
    {
        case 0:
            ADC_Channel(ADC_PHASE_A);
        break;
        case 1:
            ADC_Channel(ADC_PHASE_C);
        break;
        case 2:
            ADC_Channel(ADC_PHASE_B); 
        break;
        case 3:
            ADC_Channel(ADC_PHASE_A);
        break;
        case 4:
            ADC_Channel(ADC_PHASE_C);
        break;
        case 5:
            ADC_Channel(ADC_PHASE_B); 
        break;
    }
	return HAL_ADC_GetValue(&hadc);
}

void SM_setSwitchingDuty(uint16_t duty)
{
    //TODO: protection on max allowable duty
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, duty);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, duty);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, duty);
}


