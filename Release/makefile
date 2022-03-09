################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include startup/subdir.mk
-include pinterface/sh2_interface/subdir.mk
-include pinterface/subdir.mk
-include lib/tablet_com_protocol/test/subdir.mk
-include lib/tablet_com_protocol/subdir.mk
-include lib/stomeas_lib/subdir.mk
-include lib/ring_buffer/subdir.mk
-include lib/fifo/subdir.mk
-include lib/ds3231_rtc/subdir.mk
-include lib/bno080-driver/subdir.mk
-include board/subdir.mk
-include app/queues/streamer_service/subdir.mk
-include app/queues/sd_writer_service_queue/subdir.mk
-include app/queues/notification/subdir.mk
-include app/queues/measurements/subdir.mk
-include app/queues/config_service/subdir.mk
-include app/queues/android_device_com/subdir.mk
-include app/data/subdir.mk
-include app/config/subdir.mk
-include app/subdir.mk
-include Src/subdir.mk
-include Middlewares/Third_Party/FreeRTOS/Source/portable/MemMang/subdir.mk
-include Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM7/r0p1/subdir.mk
-include Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS/subdir.mk
-include Middlewares/Third_Party/FreeRTOS/Source/subdir.mk
-include Middlewares/Third_Party/FatFs/src/option/subdir.mk
-include Middlewares/Third_Party/FatFs/src/subdir.mk
-include Drivers/STM32H7xx_HAL_Driver/Src/subdir.mk
-include Drivers/STM32H7xx_HAL_Driver/Inc/subdir.mk
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
all: Nomade_mb_V20.elf

# Tool invocations
Nomade_mb_V20.elf: $(OBJS) $(USER_OBJS) ../STM32H743ZITx_FLASH.ld
	@echo 'Building target: $@'
	@echo 'Invoking: MCU GCC Linker'
	arm-none-eabi-gcc -mcpu=cortex-m7 -mthumb -mfloat-abi=hard -mfpu=fpv5-d16 -specs=nosys.specs -specs=nano.specs -T"../STM32H743ZITx_FLASH.ld" -Wl,-Map=output.map -Wl,--gc-sections -o "Nomade_mb_V20.elf" @"objects.list" $(USER_OBJS) $(LIBS) -lm
	@echo 'Finished building target: $@'
	@echo ' '
	$(MAKE) --no-print-directory post-build

# Other Targets
clean:
	-$(RM) *
	-@echo ' '

post-build:
	-@echo 'Generating hex and Printing size information:'
	arm-none-eabi-objcopy -O ihex "Nomade_mb_V20.elf" "Nomade_mb_V20.hex"
	arm-none-eabi-size "Nomade_mb_V20.elf"
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY: post-build

-include ../makefile.targets
