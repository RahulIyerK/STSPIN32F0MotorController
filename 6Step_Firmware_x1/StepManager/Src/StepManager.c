#include "StepManager.h"
#include "StepManager_defs.h"
#include "CurrentController.h"
#include "stm32f0xx_hal.h"
#include <stdint.h>

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;
extern ADC_HandleTypeDef hadc;

volatile uint32_t currentBemfAdcChannel;

static inline void ADC_Channel(uint32_t adc_ch)
{

  hadc.Instance->CR |= ADC_CR_ADSTP;
  while(hadc.Instance->CR & ADC_CR_ADSTP);
  /* Regular sequence configuration */
  /* Set the channel selection register from the selected channel */
  hadc.Instance->CHSELR = ADC_CHSELR_CHANNEL(adc_ch);
  hadc.Instance->CR |= ADC_CR_ADSTART;

}

volatile uint16_t duty_tracker;


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

volatile uint8_t curr_step = 0;
uint8_t alignment_index = 0;
uint8_t ramp_index = 0;
uint8_t bemf_rising = 0;
uint16_t run_arr = 0;
uint8_t arr_set = 0;
uint8_t bemf_check_cnt = 0;

uint16_t ramp_table [RAMP_TABLE_ENTRIES] = 
{
		1000,
		1000,
		500,
		500,
		500,
		500,
		500,
		250,
		200,
		131,
		100,
		80,
		70,
		62,
		55,
		50,
		46,
		43,
		40,
		38,
		36,
		34,
		32,
		30,
		28,
		27,
		26,
		25,
		24,
		23,
		22,
		21,
		20,
		20,
		20,
		20,
		20,
		20,
		20
};

uint16_t SM_fetchRampARR(uint8_t index)
{
    return ramp_table[index];
}

void setupFETs()
{
    switch (curr_step)
            {
                case 0:
                {
                    HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_B, GPIO_PIN_SET);   // set   PHASE_B GPIO
                    HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_C, GPIO_PIN_RESET); // reset PHASE_C GPIO
                    HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_A, GPIO_PIN_RESET); // reset PHASE_A GPIO
                    (&htim1)->Instance->CCR3 = duty_tracker;
                }
                break;
                case 1:
                {
                    HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_B, GPIO_PIN_SET);   // set   PHASE_B
                    HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_A, GPIO_PIN_RESET); // reset PHASE_A
                    HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_C, GPIO_PIN_RESET); // reset PHASE_C
                    (&htim1)->Instance->CCR1 = duty_tracker;

                }
                break;
                case 2:
                {
                    HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_C, GPIO_PIN_SET);   // set   PHASE_C
                    HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_A, GPIO_PIN_RESET); // reset PHASE_A
                    HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_B, GPIO_PIN_RESET); // reset PHASE_B
                    (&htim1)->Instance->CCR1 = duty_tracker;

                }
                break;
                case 3:
                {
                    HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_C, GPIO_PIN_SET);   // set   PHASE_C
                    HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_B, GPIO_PIN_RESET); // reset PHASE_B
                    HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_A, GPIO_PIN_RESET); // reset PHASE_A
                    (&htim1)->Instance->CCR2 = duty_tracker;

                }
                break;
                case 4:
                {
                    HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_A, GPIO_PIN_SET);   // set   PHASE_A
                    HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_B, GPIO_PIN_RESET); // reset PHASE_B
                    HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_C, GPIO_PIN_RESET); // reset PHASE_C
                    (&htim1)->Instance->CCR2 = duty_tracker;

                }
                break;
                case 5:
                {
                    HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_A, GPIO_PIN_SET);   // set   PHASE_A
                    HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_C, GPIO_PIN_RESET); // reset PHASE_C
                    HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_B, GPIO_PIN_RESET); // reset PHASE_B  
                    (&htim1)->Instance->CCR3 = duty_tracker;

                }
                break;

            }
}
void configStep()
{
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, 0);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, 0);
    // TODO: probably don't have to be this aggressive about turning everything off before changing steps

    if (curr_step % 2)
    {
    	bemf_rising = 1;
    }
    else
    {
    	bemf_rising = 0;
    }

    switch (controlState)
    {
        case ALIGNMENT:
        {
            // align to intermediate position before step 0
            HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_B, GPIO_PIN_SET);   // set   PHASE_B GPIO
            HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_C, GPIO_PIN_RESET); // reset PHASE_C GPIO
            HAL_GPIO_WritePin(GPIOB, LOWSIDE_PHASE_A, GPIO_PIN_RESET);   // set PHASE_A GPIO
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
            if (ramp_index < RAMP_TABLE_ENTRIES)
            {
                CC_setCurrentReference(RAMP_CURRENT_REF);
                uint16_t arr = SM_fetchRampARR(ramp_index); //set next step duration
                __HAL_TIM_SET_AUTORELOAD(&htim2, arr);
                ramp_index++;
            }
            else
            {
            	CC_setCurrentReference(RUN_START_CURRENT);
            	run_arr = SM_fetchRampARR(ramp_index);
                controlState = RUN;
            }
            setupFETs();
            

        }
        break;
        case RUN:
        {
            //TODO: set current reference based on speed control cycle output
            setupFETs();
            
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
    arr_set = 1; //initialize so that run state ARR control only starts on the first full run step
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
            CC_resetIntegral(); // TODO: how often should integral be reset?
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

static inline void updateARR(uint16_t ctr)
{
	run_arr = ctr + (run_arr >> 1);
	__HAL_TIM_SET_AUTORELOAD(&htim2, run_arr);
	arr_set = 1;
}

void SM_processBEMF(uint32_t bemf)
{
    if (controlState == RUN)
    {
    	uint16_t ctr = __HAL_TIM_GET_COUNTER(&htim2);
    	if (ctr == 0)
    	{
    		arr_set = 0;
    		bemf_check_cnt = 0;
    	}
    	if (ctr > DEMAG_NUM_PERIODS)
    	{
			if ((bemf_rising == 1)&&(arr_set == 0)) //rising BEMF
			{
				if (bemf > BEMF_ZC_THRESH)
				{
					bemf_check_cnt++;
				}
				if (bemf_check_cnt == ZC_DETECTIONS_FOR_VALID)
				{
					updateARR(ctr);
				}
			}
			if ((bemf_rising == 0)&&(arr_set == 0)) //falling BEMF
			{
				if (bemf < BEMF_ZC_THRESH)
				{
					bemf_check_cnt++;
				}
				if (bemf_check_cnt == ZC_DETECTIONS_FOR_VALID)
				{
					updateARR(ctr);
				}
			}
    	}

    }
}

void SM_updateDuty(uint16_t duty)
{
    duty_tracker = duty;
    switch (curr_step)
    {
        case 0:
            __disable_irq();
            (&htim1)->Instance->CCR3 = duty;
            __enable_irq();
        break;
        case 1:
            __disable_irq();
            (&htim1)->Instance->CCR1 = duty;
            __enable_irq();
        break;
        case 2:
        	__disable_irq();
            (&htim1)->Instance->CCR1 = duty;
            __enable_irq();
        break;
        case 3:
        	__disable_irq();
            (&htim1)->Instance->CCR2 = duty;
            __enable_irq();
        break;
        case 4:
        	__disable_irq();
            (&htim1)->Instance->CCR2 = duty;
            __enable_irq();
        break;
        case 5:
        	__disable_irq();
            (&htim1)->Instance->CCR3 = duty;
            __enable_irq();
        break;
    }
}
