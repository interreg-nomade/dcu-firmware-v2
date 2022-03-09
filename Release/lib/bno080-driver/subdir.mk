################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../lib/bno080-driver/sh2.c \
../lib/bno080-driver/sh2_SensorValue.c \
../lib/bno080-driver/sh2_util.c \
../lib/bno080-driver/shtp.c \
../lib/bno080-driver/worldTare.c 

OBJS += \
./lib/bno080-driver/sh2.o \
./lib/bno080-driver/sh2_SensorValue.o \
./lib/bno080-driver/sh2_util.o \
./lib/bno080-driver/shtp.o \
./lib/bno080-driver/worldTare.o 

C_DEPS += \
./lib/bno080-driver/sh2.d \
./lib/bno080-driver/sh2_SensorValue.d \
./lib/bno080-driver/sh2_util.d \
./lib/bno080-driver/shtp.d \
./lib/bno080-driver/worldTare.d 


# Each subdirectory must supply rules for building sources it contributes
lib/bno080-driver/%.o: ../lib/bno080-driver/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Compiler'
	@echo $(PWD)
	arm-none-eabi-gcc -mcpu=cortex-m7 -mthumb -mfloat-abi=hard -mfpu=fpv5-d16 '-D__weak=__attribute__((weak))' '-D__packed="__attribute__((__packed__))"' -DUSE_HAL_DRIVER -DSTM32H743xx -I"C:/Users/Sarah/workspace/Nomade_mb_V20/Inc" -I"C:/Users/Sarah/workspace/Nomade_mb_V20/Drivers/STM32H7xx_HAL_Driver/Inc" -I"C:/Users/Sarah/workspace/Nomade_mb_V20/Drivers/STM32H7xx_HAL_Driver/Inc/Legacy" -I"C:/Users/Sarah/workspace/Nomade_mb_V20/Middlewares/Third_Party/FatFs/src" -I"C:/Users/Sarah/workspace/Nomade_mb_V20/Middlewares/Third_Party/FreeRTOS/Source/include" -I"C:/Users/Sarah/workspace/Nomade_mb_V20/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS" -I"C:/Users/Sarah/workspace/Nomade_mb_V20/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM7/r0p1" -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -I"C:/Users/Sarah/workspace/Nomade_mb_V20/Drivers/CMSIS/Device/ST/STM32H7xx/Include" -I"C:/Users/Sarah/workspace/Nomade_mb_V20/Drivers/CMSIS/Include"  -Og -g3 -Wall -fmessage-length=0 -ffunction-sections -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


