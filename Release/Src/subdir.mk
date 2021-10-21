################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Src/UartRingbuffer.c \
../Src/UartRingbufferManager.c \
../Src/bsp_driver_sd.c \
../Src/fatfs.c \
../Src/freertos.c \
../Src/gpio.c \
../Src/i2c.c \
../Src/imu_com.c \
../Src/main.c \
../Src/mdma.c \
../Src/proteusII_driver.c \
../Src/rtc.c \
../Src/sd_card_com.c \
../Src/sd_diskio.c \
../Src/sdmmc.c \
../Src/stm32h7xx_hal_msp.c \
../Src/stm32h7xx_hal_timebase_tim.c \
../Src/stm32h7xx_it.c \
../Src/syscall.c \
../Src/system_stm32h7xx.c \
../Src/tim.c \
../Src/uart_com.c \
../Src/usart.c \
../Src/usb_com.c 

OBJS += \
./Src/UartRingbuffer.o \
./Src/UartRingbufferManager.o \
./Src/bsp_driver_sd.o \
./Src/fatfs.o \
./Src/freertos.o \
./Src/gpio.o \
./Src/i2c.o \
./Src/imu_com.o \
./Src/main.o \
./Src/mdma.o \
./Src/proteusII_driver.o \
./Src/rtc.o \
./Src/sd_card_com.o \
./Src/sd_diskio.o \
./Src/sdmmc.o \
./Src/stm32h7xx_hal_msp.o \
./Src/stm32h7xx_hal_timebase_tim.o \
./Src/stm32h7xx_it.o \
./Src/syscall.o \
./Src/system_stm32h7xx.o \
./Src/tim.o \
./Src/uart_com.o \
./Src/usart.o \
./Src/usb_com.o 

C_DEPS += \
./Src/UartRingbuffer.d \
./Src/UartRingbufferManager.d \
./Src/bsp_driver_sd.d \
./Src/fatfs.d \
./Src/freertos.d \
./Src/gpio.d \
./Src/i2c.d \
./Src/imu_com.d \
./Src/main.d \
./Src/mdma.d \
./Src/proteusII_driver.d \
./Src/rtc.d \
./Src/sd_card_com.d \
./Src/sd_diskio.d \
./Src/sdmmc.d \
./Src/stm32h7xx_hal_msp.d \
./Src/stm32h7xx_hal_timebase_tim.d \
./Src/stm32h7xx_it.d \
./Src/syscall.d \
./Src/system_stm32h7xx.d \
./Src/tim.d \
./Src/uart_com.d \
./Src/usart.d \
./Src/usb_com.d 


# Each subdirectory must supply rules for building sources it contributes
Src/%.o: ../Src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Compiler'
	@echo $(PWD)
	arm-none-eabi-gcc -mcpu=cortex-m7 -mthumb -mfloat-abi=hard -mfpu=fpv5-d16 '-D__weak=__attribute__((weak))' '-D__packed="__attribute__((__packed__))"' -DUSE_HAL_DRIVER -DSTM32H743xx -I"C:/Users/Sarah/Documents/KULeuven/NOMADe/educat-mainboard-v3-firmware-master_IMU/Inc" -I"C:/Users/Sarah/Documents/KULeuven/NOMADe/educat-mainboard-v3-firmware-master_IMU/Drivers/STM32H7xx_HAL_Driver/Inc" -I"C:/Users/Sarah/Documents/KULeuven/NOMADe/educat-mainboard-v3-firmware-master_IMU/Drivers/STM32H7xx_HAL_Driver/Inc/Legacy" -I"C:/Users/Sarah/Documents/KULeuven/NOMADe/educat-mainboard-v3-firmware-master_IMU/Middlewares/Third_Party/FatFs/src" -I"C:/Users/Sarah/Documents/KULeuven/NOMADe/educat-mainboard-v3-firmware-master_IMU/Middlewares/Third_Party/FreeRTOS/Source/include" -I"C:/Users/Sarah/Documents/KULeuven/NOMADe/educat-mainboard-v3-firmware-master_IMU/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS" -I"C:/Users/Sarah/Documents/KULeuven/NOMADe/educat-mainboard-v3-firmware-master_IMU/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM7/r0p1" -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -I"C:/Users/Sarah/Documents/KULeuven/NOMADe/educat-mainboard-v3-firmware-master_IMU/Drivers/CMSIS/Device/ST/STM32H7xx/Include" -I"C:/Users/Sarah/Documents/KULeuven/NOMADe/educat-mainboard-v3-firmware-master_IMU/Drivers/CMSIS/Include"  -Og -g3 -Wall -fmessage-length=0 -ffunction-sections -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


