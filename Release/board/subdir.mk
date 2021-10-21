################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../board/board.c 

OBJS += \
./board/board.o 

C_DEPS += \
./board/board.d 


# Each subdirectory must supply rules for building sources it contributes
board/%.o: ../board/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Compiler'
	@echo $(PWD)
	arm-none-eabi-gcc -mcpu=cortex-m7 -mthumb -mfloat-abi=hard -mfpu=fpv5-d16 '-D__weak=__attribute__((weak))' '-D__packed="__attribute__((__packed__))"' -DUSE_HAL_DRIVER -DSTM32H743xx -I"C:/Users/Sarah/Documents/KULeuven/NOMADe/educat-mainboard-v3-firmware-master_IMU/Inc" -I"C:/Users/Sarah/Documents/KULeuven/NOMADe/educat-mainboard-v3-firmware-master_IMU/Drivers/STM32H7xx_HAL_Driver/Inc" -I"C:/Users/Sarah/Documents/KULeuven/NOMADe/educat-mainboard-v3-firmware-master_IMU/Drivers/STM32H7xx_HAL_Driver/Inc/Legacy" -I"C:/Users/Sarah/Documents/KULeuven/NOMADe/educat-mainboard-v3-firmware-master_IMU/Middlewares/Third_Party/FatFs/src" -I"C:/Users/Sarah/Documents/KULeuven/NOMADe/educat-mainboard-v3-firmware-master_IMU/Middlewares/Third_Party/FreeRTOS/Source/include" -I"C:/Users/Sarah/Documents/KULeuven/NOMADe/educat-mainboard-v3-firmware-master_IMU/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS" -I"C:/Users/Sarah/Documents/KULeuven/NOMADe/educat-mainboard-v3-firmware-master_IMU/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM7/r0p1" -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -I"C:/Users/Sarah/Documents/KULeuven/NOMADe/educat-mainboard-v3-firmware-master_IMU/Drivers/CMSIS/Device/ST/STM32H7xx/Include" -I"C:/Users/Sarah/Documents/KULeuven/NOMADe/educat-mainboard-v3-firmware-master_IMU/Drivers/CMSIS/Include"  -Og -g3 -Wall -fmessage-length=0 -ffunction-sections -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


