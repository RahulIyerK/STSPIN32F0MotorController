#include "StepManager.h"
#include "stm32f0xx_hal_tim.h"
#include "stm32f0xx_hal_gpio.h"
#include "stm32f0xx_hal_adc.h"

extern TIM_HandleTypeDef htim1;
extern ADC_HandleTypeDef hadc;

typedef enum PHASE_E
{
    PHASE_A,
    PHASE_B,
    PHASE_C
} PHASE_E;


typedef struct Step_Config_S
{
    PHASE_E high_phase;
    PHASE_E low_phase;
    PHASE_E open_phase;
} Step_Config_S;


Step_Config_S Step_Table[6] = 
{
    {PHASE_C, PHASE_B, PHASE_A},
    {PHASE_A, PHASE_B, PHASE_C},
    {PHASE_A, PHASE_C, PHASE_B},
    {PHASE_B, PHASE_C, PHASE_A},
    {PHASE_B, PHASE_A, PHASE_C},
    {PHASE_C, PHASE_A, PHASE_B}   
};

uint16_t LowSide_GPIO_Lookup [3] = {GPIO_PIN_13, GPIO_PIN_14, GPIO_PIN_15};
uint16_t HighSide_Channel_Lookup [3] = {TIM_CHANNEL_1, TIM_CHANNEL_2, TIM_CHANNEL_3};
uint32_t ADC_Channel_Lookup [3] = {ADC_CHANNEL_0, ADC_CHANNEL_1, ADC_CHANNEL_2};
int curr_step = 0;

void configStep()
{
    
    // TODO: probably don't have to be this aggressive about turning everything off before changing steps

    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, 0);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, 0);

    HAL_TIM_PWM_Stop(&htim1,TIM_CHANNEL_1);
    HAL_TIM_PWM_Stop(&htim1,TIM_CHANNEL_2);
    HAL_TIM_PWM_Stop(&htim1,TIM_CHANNEL_3);

    HAL_GPIO_WritePin(GPIOB, LowSide_GPIO_Lookup[Step_Table[curr_step].low_phase], GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOB, LowSide_GPIO_Lookup[Step_Table[curr_step].high_phase], GPIO_PIN_RESET);

    HAL_TIM_PWM_Start(&htim1, HighSide_Channel_Lookup[Step_Table[curr_step].high_phase]);

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
	ADC_Channel(ADC_Channel_Lookup[Step_Table[curr_step].open_phase]);
	return HAL_ADC_GetValue(&hadc);
}

void SM_setSwitchingDuty(uint16_t duty)
{
    //TODO: protection on max allowable duty

    __HAL_TIM_SET_COMPARE(&htim1, HighSide_Channel_Lookup[Step_Table[curr_step].high_phase], duty);
}


