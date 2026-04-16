################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
C:/Users/klima/OneDrive\ -\ Kaunas\ University\ of\ Technology/stm_sensor_temp_1/STMFSMmodel/src-gen/Statechart.c 

OBJS += \
./Core/src-gen/Statechart.o 

C_DEPS += \
./Core/src-gen/Statechart.d 


# Each subdirectory must supply rules for building sources it contributes
Core/src-gen/Statechart.o: C:/Users/klima/OneDrive\ -\ Kaunas\ University\ of\ Technology/stm_sensor_temp_1/STMFSMmodel/src-gen/Statechart.c Core/src-gen/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F411xE -c -I../Core/Inc -I"C:/Users/klima/OneDrive - Kaunas University of Technology/stm_sensor_temp_1/stmIdeCode/STM_Monitor/Drivers/OLED/ssd1306" -I"C:/Users/klima/OneDrive - Kaunas University of Technology/stm_sensor_temp_1/STMFSMmodel" -I"C:/Users/klima/OneDrive - Kaunas University of Technology/stm_sensor_temp_1/STMFSMmodel/src-gen" -I"C:/Users/klima/OneDrive - Kaunas University of Technology/stm_sensor_temp_1/STMFSMmodel/src" -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-src-2d-gen

clean-Core-2f-src-2d-gen:
	-$(RM) ./Core/src-gen/Statechart.cyclo ./Core/src-gen/Statechart.d ./Core/src-gen/Statechart.o ./Core/src-gen/Statechart.su

.PHONY: clean-Core-2f-src-2d-gen

