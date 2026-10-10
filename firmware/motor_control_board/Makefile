TARGET   := Motor_Control_Board
BUILD    := build

MAKEFLAGS += -j$(shell nproc)

# Leave empty if arm-none-eabi-* is on PATH, else e.g. TOOLCHAIN=/path/to/bin/
TOOLCHAIN ?=
CC      := $(TOOLCHAIN)arm-none-eabi-gcc
OBJCOPY := $(TOOLCHAIN)arm-none-eabi-objcopy
SIZE    := $(TOOLCHAIN)arm-none-eabi-size

# Sources
C_SRCS   := $(wildcard Core/Src/*.c) $(wildcard Drivers/STM32F1xx_HAL_Driver/Src/*.c)
ASM_SRCS := Core/Startup/startup_stm32f103rctx.s
LDSCRIPT := STM32F103RCTX_FLASH.ld

INCLUDES := -ICore/Inc \
            -IDrivers/STM32F1xx_HAL_Driver/Inc \
            -IDrivers/STM32F1xx_HAL_Driver/Inc/Legacy \
            -IDrivers/CMSIS/Device/ST/STM32F1xx/Include \
            -IDrivers/CMSIS/Include
DEFINES  := -DUSE_HAL_DRIVER -DSTM32F103xE

# Flags
MCU     := -mcpu=cortex-m3 -mthumb -mfloat-abi=soft
OPT     ?= -O0 -g3 -DDEBUG
CFLAGS  := $(MCU) -std=gnu11 $(OPT) $(DEFINES) $(INCLUDES) \
           -ffunction-sections -fdata-sections -Wall --specs=nano.specs -MMD -MP
ASFLAGS := $(MCU) -g3 -x assembler-with-cpp --specs=nano.specs -MMD -MP
LDFLAGS := $(MCU) -T$(LDSCRIPT) --specs=nosys.specs --specs=nano.specs -static \
           -Wl,-Map=$(BUILD)/$(TARGET).map -Wl,--gc-sections \
           -Wl,--start-group -lc -lm -Wl,--end-group

OBJS := $(C_SRCS:%.c=$(BUILD)/%.o) $(ASM_SRCS:%.s=$(BUILD)/%.o)

# Targets
all: $(BUILD)/$(TARGET).bin $(BUILD)/$(TARGET).hex
	$(SIZE) $(BUILD)/$(TARGET).elf

$(BUILD)/$(TARGET).elf: $(OBJS) $(LDSCRIPT)
	$(CC) $(OBJS) $(LDFLAGS) -o $@

$(BUILD)/%.bin: $(BUILD)/%.elf
	$(OBJCOPY) -O binary $< $@

$(BUILD)/%.hex: $(BUILD)/%.elf
	$(OBJCOPY) -O ihex $< $@

$(BUILD)/%.o: %.c Makefile
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/%.o: %.s Makefile
	@mkdir -p $(dir $@)
	$(CC) $(ASFLAGS) -c $< -o $@

flash: $(BUILD)/$(TARGET).bin
	sudo stm32flash -w $< -v -g 0x08000000 /dev/ttyUSB0

clean:
	rm -rf $(BUILD)

-include $(OBJS:.o=.d)

.PHONY: all flash clean