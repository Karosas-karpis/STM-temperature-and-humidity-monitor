################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../FSM/Statechart.c 

OBJS += \
./FSM/Statechart.o 

C_DEPS += \
./FSM/Statechart.d 


# Each subdirectory must supply rules for building sources it contributes
FSM/%.o FSM/%.su FSM/%.cyclo: ../FSM/%.c FSM/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F401xC -c -I"C:/Users/klima/Documents/GitHub/stm-401-pcb/STM-401/FSM" -I../Core/Inc -I"C:/Users/klima/Documents/GitHub/stm-401-pcb/STM-401/Drivers/OLED/ssd1306" -I"C:/Users/klima/Documents/GitHub/stm-401-pcb/STM-401/FSM" -I"C:/Users/klima/Documents/GitHub/stm-401-pcb/STM-401/Drivers/OLED" -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-FSM

clean-FSM:
	-$(RM) ./FSM/Statechart.cyclo ./FSM/Statechart.d ./FSM/Statechart.o ./FSM/Statechart.su

.PHONY: clean-FSM

