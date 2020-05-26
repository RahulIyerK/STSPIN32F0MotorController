#include "StepManager.h"
#include "StepManager_defs.h"
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

typedef enum CONTROL_STATE_E
{
    ALIGNMENT,  // rotor alignment
    STEPPER,    // stepper drive (12-steps, rotor forced to align with each step)
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

extern TIM_HandleTypeDef htim2;

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
    switch (controlState)
    {
        case ALIGNMENT:
        {
            // align to intermediate position before step 0
            __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0);
            __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, 0);
            __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, 0);
            HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_B, GPIO_PIN_SET);   // set   PHASE_B GPIO
            HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_C, GPIO_PIN_RESET); // reset PHASE_C GPIO
            HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_A, GPIO_PIN_SET);   // set PHASE_A GPIO
            __HAL_TIM_SET_COMPARE(&htim1, HIGHSIDE_PHASE_C, duty);     // start PHASE_C PWM
        }
        break;
        case STEPPER:
        break;
        case RAMP:
        break;
        case RUN:
        {
            // TODO: probably don't have to be this aggressive about turning everything off before changing steps

            __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0);
            __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, 0);
            __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, 0);

            switch (curr_step)
            {
                case 0:
                {
                    HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_B, GPIO_PIN_SET);   // set   PHASE_B GPIO
                    HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_C, GPIO_PIN_RESET); // reset PHASE_C GPIO
                    HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_A, GPIO_PIN_RESET); // reset PHASE_A GPIO
                    __HAL_TIM_SET_COMPARE(&htim1, HIGHSIDE_PHASE_C, duty);     // start PHASE_C PWM
                }
                break;
                case 1:
                {
                    HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_B, GPIO_PIN_SET);   // set   PHASE_B
                    HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_A, GPIO_PIN_RESET); // reset PHASE_A
                    HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_C, GPIO_PIN_RESET); // reset PHASE_C
                    __HAL_TIM_SET_COMPARE(&htim1, HIGHSIDE_PHASE_A, duty);     // start PHASE_A PWM
                }
                break;
                case 2:
                {
                    HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_C, GPIO_PIN_SET);   // set   PHASE_C
                    HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_A, GPIO_PIN_RESET); // reset PHASE_A
                    HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_B, GPIO_PIN_RESET); // reset PHASE_B
                    __HAL_TIM_SET_COMPARE(&htim1, HIGHSIDE_PHASE_A, duty);     // start PHASE_A PWM
                }
                break;
                case 3:
                {
                    HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_C, GPIO_PIN_SET);   // set   PHASE_C
                    HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_B, GPIO_PIN_RESET); // reset PHASE_B
                    HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_A, GPIO_PIN_RESET); // reset PHASE_A
                    __HAL_TIM_SET_COMPARE(&htim1, HIGHSIDE_PHASE_B, duty);     // start PHASE_B PWM
                }
                break;
                case 4:
                {
                    HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_A, GPIO_PIN_SET);   // set   PHASE_A
                    HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_B, GPIO_PIN_RESET); // reset PHASE_B
                    HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_C, GPIO_PIN_RESET); // reset PHASE_C
                    __HAL_TIM_SET_COMPARE(&htim1, HIGHSIDE_PHASE_B, duty);     // start PHASE_B PWM
                }
                break;
                case 5:
                {
                    HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_A, GPIO_PIN_SET);   // set   PHASE_A
                    HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_C, GPIO_PIN_RESET); // reset PHASE_C
                    HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_B, GPIO_PIN_RESET); // reset PHASE_B  
                    __HAL_TIM_SET_COMPARE(&htim1, HIGHSIDE_PHASE_C, duty);     // start PHASE_C PWM
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


    HAL_TIM_PWM_Start(&htim1,TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim1,TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim1,TIM_CHANNEL_3);

    __HAL_TIM_SET_COMPARE(&htim1, HIGHSIDE_PHASE_A, duty);     // set PWM duty to 0
    __HAL_TIM_SET_COMPARE(&htim1, HIGHSIDE_PHASE_B, duty);     // set PWM duty to 0
    __HAL_TIM_SET_COMPARE(&htim1, HIGHSIDE_PHASE_C, duty);     // set PWM duty to 0


    HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_A, GPIO_PIN_RESET); // open PHASE_A LS FET
    HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_B, GPIO_PIN_RESET); // open PHASE_B LS FET
    HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_C, GPIO_PIN_RESET); // open PHASE_C LS FET

    current_error = 0;
    current_error_acc = 0;
    current_reference = 0;
    bemf = 0;
    curr_step = 5;

    controlState = ALIGNMENT;


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
