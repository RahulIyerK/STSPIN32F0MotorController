################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include startup/subdir.mk
-include User/subdir.mk
-include Middlewares/UART_serial_com/subdir.mk
-include Middlewares/MC_6Step_Lib/subdir.mk
-include Drivers/STM32F0xx_HAL_Driver/subdir.mk
-include Drivers/CMSIS/subdir.mk
-include Drivers/BSP/STEVAL-SPIN3202/subdir.mk
-include Drivers/BSP/Components/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: STSPIN32F0_Uart.elf

# Tool invocations
STSPIN32F0_Uart.elf: $(OBJS) $(USER_OBJS) C:\Users\User\Downloads\ST-SPIN3202_UART\STSPIN32F0_Uart/LinkerScript.ld
	@echo 'Building target: $@'
	@echo 'Invoking: MCU GCC Linker'
	arm-none-eabi-gcc -mcpu=cortex-m0 -mthumb -mfloat-abi=soft --specs=nano.specs --specs=nosys.specs -T"C:/Users/User/Downloads/ST-SPIN3202_UART/STSPIN32F0_Uart/LinkerScript.ld" -Wl,-Map=output.map -Wl,--gc-sections -o "STSPIN32F0_Uart.elf" @"objects.list" $(USER_OBJS) $(LIBS) -lm
	@echo 'Finished building target: $@'
	@echo ' '
	$(MAKE) --no-print-directory post-build

# Other Targets
clean:
	-$(RM) *
	-@echo ' '

post-build:
	-@echo 'Generating binary and Printing size information:'
	arm-none-eabi-objcopy -O binary "STSPIN32F0_Uart.elf" "STSPIN32F0_Uart.bin"
	arm-none-eabi-size "STSPIN32F0_Uart.elf"
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY: post-build

-include ../makefile.targets
