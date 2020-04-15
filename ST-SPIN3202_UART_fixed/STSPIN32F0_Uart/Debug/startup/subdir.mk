################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
S_SRCS += \
../startup/startup_stm32f031x6.s 

OBJS += \
./startup/startup_stm32f031x6.o 


# Each subdirectory must supply rules for building sources it contributes
startup/%.o: ../startup/%.s
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Assembler'
	@echo $(PWD)
	arm-none-eabi-as -mcpu=cortex-m0 -mthumb -mfloat-abi=soft -I"C:/Users/User/Downloads/ST-SPIN3202_UART_fixed/STSPIN32F0_Uart/Lib/Projects/Multi/Examples/MotionControl/STEVAL-SPIN3202/Inc" -I"C:/Users/User/Downloads/ST-SPIN3202_UART_fixed/STSPIN32F0_Uart/Lib/Drivers/BSP/Components/stspin32f0" -I"C:/Users/User/Downloads/ST-SPIN3202_UART_fixed/STSPIN32F0_Uart/Lib/Middlewares/ST/MC_6Step_Lib/Inc" -I"C:/Users/User/Downloads/ST-SPIN3202_UART_fixed/STSPIN32F0_Uart/Lib/Drivers/STM32F0xx_HAL_Driver/Inc" -I"C:/Users/User/Downloads/ST-SPIN3202_UART_fixed/STSPIN32F0_Uart/Lib/Drivers/CMSIS/Device/ST/STM32F0xx/Include" -I"C:/Users/User/Downloads/ST-SPIN3202_UART_fixed/STSPIN32F0_Uart/Lib/Drivers/BSP/STEVAL-SPIN3202" -I"C:/Users/User/Downloads/ST-SPIN3202_UART_fixed/STSPIN32F0_Uart/Lib/Drivers/BSP/Components/Common" -I"C:/Users/User/Downloads/ST-SPIN3202_UART_fixed/STSPIN32F0_Uart/Lib/Middlewares/ST/UART_serial_com/Inc" -I"C:/Users/User/Downloads/ST-SPIN3202_UART_fixed/STSPIN32F0_Uart/Lib/Drivers/CMSIS/Include" -g -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


