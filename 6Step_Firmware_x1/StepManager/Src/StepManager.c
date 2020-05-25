#include "StepManager.h"
//#include "stm32f0xx_hal_tim.h"
//#include "stm32f0xx_hal_gpio.h"
//#include "stm32f0xx_hal_adc.h"
#include "stm32f0xx_hal.h"
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

#define ADC_CURRENT_CHANNEL (ADC_CHANNEL_4)

#define BEMF_thresh 200

uint32_t currentBemfAdcChannel;
volatile uint32_t bemf;
volatile uint32_t current_error;
volatile uint32_t current_error_acc;
volatile uint32_t current_reference;


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

extern TIM_HandleTypeDef htim2;

uint16_t duty;
uint8_t align_ok;

// ADC Channel selection function copied from ST's MC SDK
inline void ADC_Channel(uint32_t adc_ch)
{

  hadc.Instance->CR |= ADC_CR_ADSTP;
  while(hadc.Instance->CR & ADC_CR_ADSTP);
  /* Regular sequence configuration */
  /* Set the channel selection register from the selected channel */
  hadc.Instance->CHSELR = ADC_CHSELR_CHANNEL(adc_ch);
  hadc.Instance->CR |= ADC_CR_ADSTART;

}

void configStep()
{
    
    // TODO: probably don't have to be this aggressive about turning everything off before changing steps

//    HAL_TIM_PWM_Stop(&htim1,TIM_CHANNEL_1);
//    HAL_TIM_PWM_Stop(&htim1,TIM_CHANNEL_2);
//    HAL_TIM_PWM_Stop(&htim1,TIM_CHANNEL_3);

    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, 0);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, 0);

    switch (curr_step)
    {
        case 0:
            HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_B, GPIO_PIN_SET);   // set   PHASE_B GPIO
            HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_C, GPIO_PIN_RESET); // reset PHASE_C GPIO
            HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_A, GPIO_PIN_RESET); // reset PHASE_A GPIO
            __HAL_TIM_SET_COMPARE(&htim1, HIGHSIDE_PHASE_C, duty);     // start PHASE_C PWM

        break;
        case 1:
            HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_B, GPIO_PIN_SET);   // set   PHASE_B
            HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_A, GPIO_PIN_RESET); // reset PHASE_A
            HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_C, GPIO_PIN_RESET); // reset PHASE_C
            __HAL_TIM_SET_COMPARE(&htim1, HIGHSIDE_PHASE_A, duty);     // start PHASE_A PWM

        break;
        case 2:
            HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_C, GPIO_PIN_SET);   // set   PHASE_C
            HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_A, GPIO_PIN_RESET); // reset PHASE_A
            HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_B, GPIO_PIN_RESET); // reset PHASE_B
            __HAL_TIM_SET_COMPARE(&htim1, HIGHSIDE_PHASE_A, duty);     // start PHASE_A PWM

        break;
        case 3:
            HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_C, GPIO_PIN_SET);   // set   PHASE_C
            HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_B, GPIO_PIN_RESET); // reset PHASE_B
            HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_A, GPIO_PIN_RESET); // reset PHASE_A
            __HAL_TIM_SET_COMPARE(&htim1, HIGHSIDE_PHASE_B, duty);     // start PHASE_B PWM

        break;
        case 4:
            HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_A, GPIO_PIN_SET);   // set   PHASE_A
            HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_B, GPIO_PIN_RESET); // reset PHASE_B
            HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_C, GPIO_PIN_RESET); // reset PHASE_C
            __HAL_TIM_SET_COMPARE(&htim1, HIGHSIDE_PHASE_B, duty);     // start PHASE_B PWM

        break;
        case 5:
            HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_A, GPIO_PIN_SET);   // set   PHASE_A
            HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_C, GPIO_PIN_RESET); // reset PHASE_C

            if (align_ok)
            {
            	HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_B, GPIO_PIN_RESET); //reset PHASE_B if not in alignment mode
            }
            else
            {
            	HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_B, GPIO_PIN_SET); // set PHASE_B to align
            	align_ok = 1;
            }
            __HAL_TIM_SET_COMPARE(&htim1, HIGHSIDE_PHASE_C, duty);     // start PHASE_C PWM

        break;

    }

}




void SM_init()
{
    curr_step = 5; //initialize step manager state so that first step commutated to is 0

    currentBemfAdcChannel = 0;//zero out the current ADC channel


    HAL_TIM_PWM_Start(&htim1,TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim1,TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim1,TIM_CHANNEL_3);

    HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_A, GPIO_PIN_RESET); // reset PHASE_A GPIO
    HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_B, GPIO_PIN_RESET); // reset PHASE_B GPIO
    HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_C, GPIO_PIN_RESET); // reset PHASE_C GPIO

    duty = 36;

    current_error = 0;
    current_error_acc = 0;
    current_reference = 0;
    bemf = 0;
    align_ok = 0;

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

void SM_sampleBEMF()
{
    //upon zero-cross detection we should immediately set the period of the LF timer

	bemf = HAL_ADC_GetValue(&hadc);
    if(bemf < BEMF_thresh){
//		__HAL_TIM_SET_AUTORELOAD(&htim2, __HAL_TIM_GET_COUNTER(&htim2) << 1);
	}
    ADC_Channel(ADC_CURRENT_CHANNEL);

}

void SM_sampleCurrent()
{
	current_error = HAL_ADC_GetValue(&hadc) - current_reference;
	current_error_acc += current_error;
	ADC_Channel(currentBemfAdcChannel);
}

void SM_setSwitchingDuty(uint16_t duty)
{
    //TODO: protection on max allowable duty
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, duty);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, duty);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, duty);
}
