################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../app/data/operations.c \
../app/data/submsgs.c 

OBJS += \
./app/data/operations.o \
./app/data/submsgs.o 

C_DEPS += \
./app/data/operations.d \
./app/data/submsgs.d 


# Each subdirectory must supply rules for building sources it contributes
app/data/%.o: ../app/data/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Compiler'
	@echo $(PWD)
	arm-none-eabi-gcc -mcpu=cortex-m7 -mthumb -mfloat-abi=hard -mfpu=fpv5-d16 '-D__weak=__attribute__((weak))' '-D__packed="__attribute__((__packed__))"' -DUSE_HAL_DRIVER -DSTM32H743xx -I"C:/Users/Sarah/workspace/Nomade_mb_V20/Inc" -I"C:/Users/Sarah/workspace/Nomade_mb_V20/Drivers/STM32H7xx_HAL_Driver/Inc" -I"C:/Users/Sarah/workspace/Nomade_mb_V20/Drivers/STM32H7xx_HAL_Driver/Inc/Legacy" -I"C:/Users/Sarah/workspace/Nomade_mb_V20/Middlewares/Third_Party/FatFs/src" -I"C:/Users/Sarah/workspace/Nomade_mb_V20/Middlewares/Third_Party/FreeRTOS/Source/include" -I"C:/Users/Sarah/workspace/Nomade_mb_V20/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS" -I"C:/Users/Sarah/workspace/Nomade_mb_V20/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM7/r0p1" -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -I"C:/Users/Sarah/workspace/Nomade_mb_V20/Drivers/CMSIS/Device/ST/STM32H7xx/Include" -I"C:/Users/Sarah/workspace/Nomade_mb_V20/Drivers/CMSIS/Include"  -Og -g3 -Wall -fmessage-length=0 -ffunction-sections -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


