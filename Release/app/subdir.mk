################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../app/app_BT1_com.c \
../app/app_init.c \
../app/app_leds.c \
../app/app_rtc.c \
../app/app_streamer.c \
../app/app_tablet_com.c \
../app/app_terminal_com.c \
../app/common.c 

OBJS += \
./app/app_BT1_com.o \
./app/app_init.o \
./app/app_leds.o \
./app/app_rtc.o \
./app/app_streamer.o \
./app/app_tablet_com.o \
./app/app_terminal_com.o \
./app/common.o 

C_DEPS += \
./app/app_BT1_com.d \
./app/app_init.d \
./app/app_leds.d \
./app/app_rtc.d \
./app/app_streamer.d \
./app/app_tablet_com.d \
./app/app_terminal_com.d \
./app/common.d 


# Each subdirectory must supply rules for building sources it contributes
app/%.o: ../app/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Compiler'
	@echo $(PWD)
	arm-none-eabi-gcc -mcpu=cortex-m7 -mthumb -mfloat-abi=hard -mfpu=fpv5-d16 '-D__weak=__attribute__((weak))' '-D__packed="__attribute__((__packed__))"' -DUSE_HAL_DRIVER -DSTM32H743xx -I"C:/Users/Sarah/Documents/KULeuven/NOMADe/educat-mainboard-v3-firmware-master_IMU/Inc" -I"C:/Users/Sarah/Documents/KULeuven/NOMADe/educat-mainboard-v3-firmware-master_IMU/Drivers/STM32H7xx_HAL_Driver/Inc" -I"C:/Users/Sarah/Documents/KULeuven/NOMADe/educat-mainboard-v3-firmware-master_IMU/Drivers/STM32H7xx_HAL_Driver/Inc/Legacy" -I"C:/Users/Sarah/Documents/KULeuven/NOMADe/educat-mainboard-v3-firmware-master_IMU/Middlewares/Third_Party/FatFs/src" -I"C:/Users/Sarah/Documents/KULeuven/NOMADe/educat-mainboard-v3-firmware-master_IMU/Middlewares/Third_Party/FreeRTOS/Source/include" -I"C:/Users/Sarah/Documents/KULeuven/NOMADe/educat-mainboard-v3-firmware-master_IMU/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS" -I"C:/Users/Sarah/Documents/KULeuven/NOMADe/educat-mainboard-v3-firmware-master_IMU/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM7/r0p1" -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -I"C:/Users/Sarah/Documents/KULeuven/NOMADe/educat-mainboard-v3-firmware-master_IMU/Drivers/CMSIS/Device/ST/STM32H7xx/Include" -I"C:/Users/Sarah/Documents/KULeuven/NOMADe/educat-mainboard-v3-firmware-master_IMU/Drivers/CMSIS/Include"  -Og -g3 -Wall -fmessage-length=0 -ffunction-sections -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


