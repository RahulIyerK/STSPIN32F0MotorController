/**
 ******************************************************************************
 * @file    6Step_Lib.c
 * @author  IPC Rennes
 * @version V1.0.0
 * @date    April 24th, 2017
 * @brief   This file provides the set of functions for Motor Control library 
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT(c) 2017 STMicroelectronics</center></h2>
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *   1. Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright notice,
 *      this list of conditions and the following disclaimer in the documentation
 *      and/or other materials provided with the distribution.
 *   3. Neither the name of STMicroelectronics nor the names of its contributors
 *      may be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************
 */

/*! ****************************************************************************
================================================================================   
                 ###### Main functions for 6-Step algorithm ######
================================================================================     
The main function are the following:

1) MC_SixStep_TABLE(...) -> Set the peripherals (TIMx, GPIO etc.) for each step
2) MC_SixStep_ARR_step() -> Generate the ARR value for Low Frequency TIM during start-up
3) MC_SixStep_INIT()     -> Init the main variables for motor driving from MC_SixStep_param.h
4) MC_SixStep_RESET()    -> Reset all variables used for 6Step control algorithm
5) MC_SixStep_Ramp_Motor_calc() -> Calculate the acceleration profile step by step for motor during start-up 
6) MC_SixStep_NEXT_step()-> Generate the next step number according with the direction (CW or CCW)
7) MC_Task_Speed()       -> Speed Loop with PI regulator
8) MC_Set_Speed(...)     -> Set the new motor speed value
9) MC_StartMotor()       -> Start the Motor
10)MC_StopMotor()       -> Stop the Motor
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "6Step_Lib.h"

#include <string.h>

/** @addtogroup MIDDLEWARES     MIDDLEWARES 
  * @brief  Middlewares Layer
  * @{ 
  */


/** @addtogroup MC_6-STEP_LIB       MC_6-STEP LIB 
  * @brief  Motor Control driver
  * @{ 
  */

/* Data struct ---------------------------------------------------------------*/
SIXSTEP_Base_InitTypeDef SIXSTEP_parameters;            /*!< Main SixStep structure*/ 
SIXSTEP_PI_PARAM_InitTypeDef_t PI_parameters;           /*!< SixStep PI regulator structure*/ 

TIM_OC_InitTypeDef sConfig;

/* Variables -----------------------------------------------------------------*/
extern TIM_HandleTypeDef HF_TIMx;
extern TIM_HandleTypeDef LF_TIMx;
extern ADC_HandleTypeDef ADCx;
#ifdef TEST
uint8_t stop = 0;
#endif
#ifdef HALL_SENSORS
uint16_t H1, H2, H3;
uint8_t hallStatus;
#endif
#ifdef BEMF_RECORDING
#define BEMF_ARRAY_SIZE 400
uint16_t bemfArray[BEMF_ARRAY_SIZE+6];
#endif
uint16_t Rotor_poles_pairs;                         /*!<  Number of pole pairs of the motor */ 
uint32_t mech_accel_hz = 0;                         /*!<  Hz -- Mechanical acceleration rate */
uint32_t constant_k = 0;                            /*!<  1/3*mech_accel_hz */
uint32_t Time_vector_tmp = 0;                       /*!<  Startup variable  */
uint32_t Time_vector_prev_tmp = 0 ;                 /*!<  Startup variable  */
uint32_t T_single_step = 0;                         /*!<  Startup variable  */
uint32_t T_single_step_first_value = 0;             /*!<  Startup variable  */
int32_t  delta = 0;                                 /*!<  Startup variable  */
uint16_t index_array = 0;                           /*!<  Speed filter variable */
int32_t speed_tmp_array[FILTER_DEEP];               /*!<  Speed filter variable */
#ifdef POTENTIOMETER
uint32_t potentiometer_prev_speed_target;           /*!< Previous speed target for the motor */
uint32_t potentiometer_speed_target;
uint16_t potentiometer_buffer[POT_BUFFER_SIZE];     /*!<  Buffer for Potentiometer Value Filtering */
uint16_t potentiometer_buffer_index = 0;            /*!<  High-Frequency Buffer Index */
#endif
uint8_t  array_completed = FALSE;                   /*!<  Speed filter variable */
uint8_t  UART_FLAG_RECEIVE = FALSE;                 /*!<  UART commmunication flag */
#ifndef HALL_SENSORS
uint32_t ARR_LF = 0;                                /*!<  Autoreload LF TIM variable */
#endif
int32_t Mech_Speed_RPM = 0;                         /*!<  Mechanical motor speed */
int32_t El_Speed_Hz = 0;                            /*!<  Electrical motor speed */
uint16_t index_adc_chn = 0;                         /*!<  Index of ADC channel selector for measuring */
#ifdef DEMOMODE
uint16_t index_motor_run = 0;                       /*!<  Tmp variable for DEMO mode */
uint16_t test_motor_run = 1;                        /*!<  Tmp variable for DEMO mode */
#endif
uint8_t Enable_start_button = TRUE;                 /*!<  Start/stop button filter to avoid double command */
#ifndef HALL_SENSORS
uint16_t index_ARR_step = 1;                           
uint32_t n_zcr_startup = 0;
uint16_t cnt_bemf_event = 0;
uint8_t startup_bemf_failure = 0;
uint8_t lf_timer_failure = 0;
uint8_t speed_fdbk_error = 0;
uint16_t index_startup_motor = 1;
#ifdef PWM_ON_BEMF_SENSING
uint8_t zcr_on_ton = 0;
uint8_t zcr_on_ton_next = 0;
#endif
#endif
uint16_t shift_n_sqrt = 14;
static __IO uint32_t uwTick = 0;                        /*!<  Tick counter - 1msec updated */
uint16_t index_align = 1;
int32_t speed_sum_sp_filt = 0;
int32_t speed_sum_pot_filt = 0;
uint16_t index_pot_filt = 1; 
int16_t potent_filtered = 0;
uint32_t Tick_cnt = 0;  
uint32_t counter_ARR_Bemf = 0;
uint64_t constant_multiplier_tmp = 0;

/** @addtogroup MotorControl_Board_Linked_Functions MotorControl Board Linked Functions
  * @{
  */
void BSP_BOARD_FAULT_LED_ON(void);
void BSP_BOARD_FAULT_LED_OFF(void);
#ifndef VOLTAGE_MODE
void MC_SixStep_Current_Reference_Start(void);
void MC_SixStep_Current_Reference_Stop(void);
void MC_SixStep_Current_Reference_Setvalue(uint16_t);
#endif
void MC_SixStep_EnableInput_CH1_E_CH2_E_CH3_D(uint8_t);
void MC_SixStep_EnableInput_CH1_E_CH2_D_CH3_E(uint8_t);
void MC_SixStep_EnableInput_CH1_D_CH2_E_CH3_E(uint8_t);
void MC_SixStep_DisableInput_CH1_D_CH2_D_CH3_D(void);
void MC_SixStep_HF_TIMx_SetDutyCycle_CH1(uint16_t);
void MC_SixStep_HF_TIMx_SetDutyCycle_CH2(uint16_t);
void MC_SixStep_HF_TIMx_SetDutyCycle_CH3(uint16_t);
#ifdef VOLTAGE_MODE 
void MC_SixStep_HF_TIMx_SetDutyCycle(uint16_t, uint8_t);
#endif
void MC_SixStep_ADC_Channel(uint32_t);
/**
  * @} end MotorControl_Board_Linked_Functions
  */

/** @addtogroup UART_UI  UART UI
  * @brief  Serial communication through PC serial terminal
  * @{ 
  */ 
#ifdef UART_COMM
void MC_UI_INIT(void);
void UART_Send_Bemf(uint16_t *, uint16_t);
void UART_Send_Speed(void);
void UART_Set_Value(void);
void UART_Communication_Task(void);
void CMD_Parser(char* pCommandString);
#endif
/**
  * @} end UART_UI
  */

/** @defgroup MC_6-STEP_LIB_Exported_Functions MC_6-STEP LIB Exported Functions
  * @{
  */ 
void MC_Set_PI_param(SIXSTEP_PI_PARAM_InitTypeDef_t *);
#ifdef HALL_SENSORS
void MC_SixStep_Hall_Startup_Failure_Handler(void);
void MC_SixStep_Hall_Run_Failure_Handler(void);
void MC_TIMx_SixStep_CommutationEvent(void);
#else
void MC_ADCx_SixStep_Bemf(void);
#ifdef PWM_ON_BEMF_SENSING
void MC_Update_ADC_Ch(uint8_t current_is_BEMF);
#endif
#endif
void MC_TIMx_SixStep_timebase(void);
void MC_SysTick_SixStep_MediumFrequencyTask(void);
void MC_SixStep_TABLE(uint8_t);


uint32_t test_ADC_read(){
	 MC_SixStep_ADC_Channel(ADC_CHANNEL_0);
	uint32_t bemf = HAL_ADC_GetValue(&ADCx);
	return bemf;
}

void MC_SixStep_INIT()
{
    BSP_SIP_HF_TIMx_IT_BRK_ENABLE();
//    MC_SixStep_Init_main_data();

    SIXSTEP_parameters.pulse_value  = HF_TIMx.Instance->CCR1;

    SIXSTEP_parameters.LF_TIMx_ARR = LF_TIMx.Init.Period;
    SIXSTEP_parameters.LF_TIMx_PSC = LF_TIMx.Init.Prescaler;
    SIXSTEP_parameters.HF_TIMx_ARR = HF_TIMx.Init.Period;
    SIXSTEP_parameters.HF_TIMx_PSC = HF_TIMx.Init.Prescaler;


  #ifdef UART_COMM
    SIXSTEP_parameters.Button_ready = FALSE;
    SIXSTEP_parameters.UART_MEASUREMENT_TYPE = 0;
    SIXSTEP_parameters.UART_CONTINUOUS_TX_BEMF_MODE = FALSE;
    SIXSTEP_parameters.UART_CONTINUOUS_TX_BEMF_ALLOWED = FALSE;
    SIXSTEP_parameters.UART_CONTINUOUS_TX_SPEED_MODE = FALSE;
    SIXSTEP_parameters.UART_CONTINUOUS_TX_SPEED_ALLOWED = FALSE;
    SIXSTEP_parameters.UART_TX_REPLY = FALSE;
    SIXSTEP_parameters.UART_TX_DIFFERED_REPLY = FALSE;
    SIXSTEP_parameters.UART_TX_CANCELLED = 0;
    MC_UI_INIT();               /*!<  Start the UART Communication Task*/
  #endif

  #ifndef UART_COMM
    SIXSTEP_parameters.Button_ready = TRUE;
  #endif
//    MC_SixStep_RESET();
}

