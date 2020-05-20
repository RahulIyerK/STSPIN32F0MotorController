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

volatile uint32_t current_buffer[3] = {0};//currently tim 3 is set to call the current controller every 2 cycles of tim1 soo we should only need to store 2 readings
volatile uint32_t bemf_buffer[3] = {0};//still using 3 just to be safe
volatile uint8_t current_index;
volatile uint8_t bemf_index;

extern TIM_HandleTypeDef htim14;

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
            HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_A, GPIO_PIN_RESET); // reset PHASE_A GPIO
            HAL_TIM_PWM_Start(&htim1, HIGHSIDE_PHASE_C);               // start PHASE_C PWM
        break;
        case 1:
            HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_B, GPIO_PIN_SET);   // set   PHASE_B
            HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_A, GPIO_PIN_RESET); // reset PHASE_A
            HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_C, GPIO_PIN_RESET); // reset PHASE_C
            HAL_TIM_PWM_Start(&htim1, HIGHSIDE_PHASE_A);               // start PHASE_A PWM
        break;
        case 2:
            HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_C, GPIO_PIN_SET);   // set   PHASE_C
            HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_A, GPIO_PIN_RESET); // reset PHASE_A
            HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_B, GPIO_PIN_RESET); // reset PHASE_B
            HAL_TIM_PWM_Start(&htim1, HIGHSIDE_PHASE_A);               // start PHASE_A PWM  
        break;
        case 3:
            HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_C, GPIO_PIN_SET);   // set   PHASE_C
            HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_B, GPIO_PIN_RESET); // reset PHASE_B
            HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_A, GPIO_PIN_RESET); // reset PHASE_A
            HAL_TIM_PWM_Start(&htim1, HIGHSIDE_PHASE_B);               // start PHASE_B PWM       
        break;
        case 4:
            HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_A, GPIO_PIN_SET);   // set   PHASE_A
            HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_B, GPIO_PIN_RESET); // reset PHASE_B
            HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_C, GPIO_PIN_RESET); // reset PHASE_C
            HAL_TIM_PWM_Start(&htim1, HIGHSIDE_PHASE_B);               // start PHASE_B PWM 
        break;
        case 5:
            HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_A, GPIO_PIN_SET);   // set   PHASE_A
            HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_C, GPIO_PIN_RESET); // reset PHASE_C
            HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_B, GPIO_PIN_RESET); // reset PHASE_B
            HAL_TIM_PWM_Start(&htim1, HIGHSIDE_PHASE_C);               // start PHASE_C PWM       
        break;

    }
}

// ADC Channel selection function copied from ST's MC SDK
void ADC_Channel(uint32_t adc_ch)
{ 
  hadc.Instance->CR |= ADC_CR_ADSTP;
  while(hadc.Instance->CR & ADC_CR_ADSTP);   
  /* Regular sequence configuration */
  /* Set the channel selection register from the selected channel */
  hadc.Instance->CHSELR = ADC_CHSELR_CHANNEL(adc_ch);
  hadc.Instance->CR |= ADC_CR_ADSTART;
}    


void SM_init()
{
    curr_step = 5; //initialize step manager state so that first step is 0

    currentBemfAdcChannel = 0;//zero out the current ADC channel
	current_index = 0;
	bemf_index = 0;

    HAL_TIM_PWM_Stop(&htim1,TIM_CHANNEL_1);
    HAL_TIM_PWM_Stop(&htim1,TIM_CHANNEL_2);
    HAL_TIM_PWM_Stop(&htim1,TIM_CHANNEL_3);

    SM_setSwitchingDuty(10);

    //TODO: rotor alignment logic...
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
            //ADC_Channel(ADC_PHASE_A);
        break;
        case 1:
            //ADC_Channel(ADC_PHASE_C);
            currentBemfAdcChannel = ADC_PHASE_C;
        break;
        case 2:
            //ADC_Channel(ADC_PHASE_B); 
            currentBemfAdcChannel = ADC_PHASE_B;
        break;
        case 3:
            //ADC_Channel(ADC_PHASE_A);
            currentBemfAdcChannel = ADC_PHASE_A;
        break;
        case 4:
            //ADC_Channel(ADC_PHASE_C);
            currentBemfAdcChannel = ADC_PHASE_C;
        break;
        case 5:
            //ADC_Channel(ADC_PHASE_B); 
            currentBemfAdcChannel = ADC_PHASE_B;
        break;
    }
}

void SM_sampleBEMF()
{
	//storing this just in case but I don't think we need it
    //upon 0 detection we should immediately set the period of the LF timer
    bemf_buffer[bemf_index] = HAL_ADC_GetValue(&hadc);

    if(bemf_buffer[bemf_index] < BEMF_thresh){
		__HAL_TIM_SET_AUTORELOAD(&htim14, __HAL_TIM_GET_COUNTER(&htim14) << 2);
	}
	bemf_index++;

}

void SM_sampleCurrent()
{
    //ADC_Channel(ADC_CURRENT_CHANNEL);
	current_buffer[current_index] = HAL_ADC_GetValue(&hadc);
	current_index++;
}

void SM_setSwitchingDuty(uint16_t duty)
{
    //TODO: protection on max allowable duty
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, duty);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, duty);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, duty);
}


