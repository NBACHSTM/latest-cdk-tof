################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include Src/ota_pal/subdir.mk
-include Src/fs/subdir.mk
-include Src/crypto/subdir.mk
-include Src/subdir.mk
-include Middlewares/ST/STM32_AI_Runtime/subdir.mk
-include Libraries/tinycbor/subdir.mk
-include Libraries/ota/portable/os/subdir.mk
-include Libraries/ota/subdir.mk
-include Libraries/mbedtls/library/subdir.mk
-include Libraries/lwip/netif/subdir.mk
-include Libraries/lwip/core/ipv6/subdir.mk
-include Libraries/lwip/core/ipv4/subdir.mk
-include Libraries/lwip/core/subdir.mk
-include Libraries/lwip/apps/lwiperf/subdir.mk
-include Libraries/lwip/api/subdir.mk
-include Libraries/freertos_kernel/port/subdir.mk
-include Libraries/freertos_kernel/subdir.mk
-include Libraries/deviceShadow/subdir.mk
-include Libraries/deviceDefender/subdir.mk
-include Libraries/corePKCS11/mbedtls_utils/subdir.mk
-include Libraries/corePKCS11/subdir.mk
-include Libraries/coreMQTTAgent/subdir.mk
-include Libraries/coreMQTT/subdir.mk
-include Libraries/coreJSON/subdir.mk
-include Libraries/backoffAlgorithm/subdir.mk
-include Libraries/STM32U5_HAL/subdir.mk
-include Libraries/LittleFS/bd/subdir.mk
-include Libraries/LittleFS/subdir.mk
-include Libraries/AudioPreProcessing/Src/subdir.mk
-include Drivers/vl53l5cx/porting/subdir.mk
-include Drivers/vl53l5cx/modules/subdir.mk
-include Drivers/vl53l5cx/subdir.mk
-include Drivers/lsm6dso/subdir.mk
-include Drivers/lps22hh/subdir.mk
-include Drivers/ism330dhcx/subdir.mk
-include Drivers/iis2mdc/subdir.mk
-include Drivers/hts221/subdir.mk
-include Drivers/bsp/subdir.mk
-include Common/sys/subdir.mk
-include Common/net/mxchip/subdir.mk
-include Common/net/lwip_port/src/subdir.mk
-include Common/net/subdir.mk
-include Common/kvstore/subdir.mk
-include Common/crypto/subdir.mk
-include Common/cli/subdir.mk
-include Common/boards/subdir.mk
-include Common/app/ota/subdir.mk
-include Common/app/mqtt/subdir.mk
-include Common/app/Target/subdir.mk
-include Common/app/subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

OPTIONAL_TOOL_DEPS := \
$(wildcard ../makefile.defs) \
$(wildcard ../makefile.init) \
$(wildcard ../makefile.targets) \


BUILD_ARTIFACT_NAME := b_u585i_iot02a_ntz
BUILD_ARTIFACT_EXTENSION := elf
BUILD_ARTIFACT_PREFIX :=
BUILD_ARTIFACT := $(BUILD_ARTIFACT_PREFIX)$(BUILD_ARTIFACT_NAME)$(if $(BUILD_ARTIFACT_EXTENSION),.$(BUILD_ARTIFACT_EXTENSION),)

# Add inputs and outputs from these tool invocations to the build variables 
EXECUTABLES += \
b_u585i_iot02a_ntz.elf \

MAP_FILES += \
b_u585i_iot02a_ntz.map \

SIZE_OUTPUT += \
default.size.stdout \

OBJDUMP_LIST += \
b_u585i_iot02a_ntz.list \

OBJCOPY_HEX += \
b_u585i_iot02a_ntz.hex \

OBJCOPY_BIN += \
b_u585i_iot02a_ntz.bin \


# All Target
all: main-build

# Main-build Target
main-build: b_u585i_iot02a_ntz.elf secondary-outputs

# Tool invocations
b_u585i_iot02a_ntz.elf b_u585i_iot02a_ntz.map: $(OBJS) $(USER_OBJS) C:/Users/bachirin/Desktop/AWS_gs/aws_poc_aed/Projects/b_u585i_iot02a_ntz/STM32U585AIIXQ_FLASH.ld makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-gcc -o "b_u585i_iot02a_ntz.elf" @"objects.list" $(USER_OBJS) $(LIBS) -mcpu=cortex-m33 -T"C:/Users/bachirin/Desktop/AWS_gs/aws_poc_aed/Projects/b_u585i_iot02a_ntz/STM32U585AIIXQ_FLASH.ld" --specs=nosys.specs -Wl,-Map="b_u585i_iot02a_ntz.map" -Wl,--cref -Wl,--gc-sections -static -L../../../Drivers/CMSIS/DSP/Lib/GCC -L../../../Middleware/STM32_AI_Library/Lib -L"C:\Users\bachirin\Desktop\AWS_gs\aws_poc_aed\Projects\b_u585i_iot02a_ntz\Middlewares\ST\STM32_AI_Runtime\Inc\lib" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -u _printf_float -Wl,--start-group -lc -lm -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

default.size.stdout: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-size  $(EXECUTABLES)
	@echo 'Finished building: $@'
	@echo ' '

b_u585i_iot02a_ntz.list: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objdump -h -S $(EXECUTABLES) > "b_u585i_iot02a_ntz.list"
	@echo 'Finished building: $@'
	@echo ' '

b_u585i_iot02a_ntz.hex: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objcopy  -O ihex $(EXECUTABLES) "b_u585i_iot02a_ntz.hex"
	@echo 'Finished building: $@'
	@echo ' '

b_u585i_iot02a_ntz.bin: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objcopy  -O binary $(EXECUTABLES) "b_u585i_iot02a_ntz.bin"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) b_u585i_iot02a_ntz.bin b_u585i_iot02a_ntz.elf b_u585i_iot02a_ntz.hex b_u585i_iot02a_ntz.list b_u585i_iot02a_ntz.map default.size.stdout
	-@echo ' '

secondary-outputs: $(SIZE_OUTPUT) $(OBJDUMP_LIST) $(OBJCOPY_HEX) $(OBJCOPY_BIN)

fail-specified-linker-script-missing:
	@echo 'Error: Cannot find the specified linker script. Check the linker settings in the build configuration.'
	@exit 2

warn-no-linker-script-specified:
	@echo 'Warning: No linker script specified. Check the linker settings in the build configuration.'

.PHONY: all clean dependents main-build fail-specified-linker-script-missing warn-no-linker-script-specified

-include ../makefile.targets
