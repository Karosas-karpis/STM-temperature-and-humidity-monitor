################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
S_SRCS += \
../Core/Startup/startup_stm32f411ceux.s 

OBJS += \
./Core/Startup/startup_stm32f411ceux.o 

S_DEPS += \
./Core/Startup/startup_stm32f411ceux.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Startup/%.o: ../Core/Startup/%.s Core/Startup/subdir.mk
	arm-none-eabi-gcc -mcpu=cortex-m4 -g3 -DDEBUG -c -I"C:/Users/darbo/Documents/GitHub/STM-temperature-and-humidity-monitor/STM_Monitor/Drivers/OLED/ssd1306" -I"C:/Users/darbo/Documents/GitHub/STM-temperature-and-humidity-monitor/STM_Monitor/FSM" -I"C:/Users/darbo/Documents/GitHub/STM-temperature-and-humidity-monitor/STMFSMmodel/src-gen" -I"C:/Users/darbo/Documents/GitHub/STM-temperature-and-humidity-monitor/STMFSMmodel/src" -I"C:/Users/darbo/Documents/GitHub/STM-temperature-and-humidity-monitor/STM_Monitor/Drivers/STM32F4xx_HAL_Driver/Inc" -I"C:/Users/darbo/Documents/GitHub/STM-temperature-and-humidity-monitor/STM_Monitor/Drivers/STM32F4xx_HAL_Driver/Src" -I"C:/Users/klima/Documents/GitHub/STM-temperature-and-humidity-monitor/STMFSMmodel/src-gen" -I"C:/Users/klima/Documents/GitHub/STM-temperature-and-humidity-monitor/STMFSMmodel/src" -I"C:/Users/darbo/Documents/GitHub/STM-temperature-and-humidity-monitor/STM_Monitor/Drivers/OLED" -x assembler-with-cpp -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@" "$<"

clean: clean-Core-2f-Startup

clean-Core-2f-Startup:
	-$(RM) ./Core/Startup/startup_stm32f411ceux.d ./Core/Startup/startup_stm32f411ceux.o

.PHONY: clean-Core-2f-Startup

