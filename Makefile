# This makefile is for libopencm3 based STM32 projects
# To use HAL/LL, see the HAL based project's makefile


TARGET=main


TOOLCHAIN_PREFIX=/home/rik/Toolchains/gcc-arm-none-eabi-10-2020-q4-major/bin/arm-none-eabi

CC=$(TOOLCHAIN_PREFIX)-gcc
LD=$(TOOLCHAIN_PREFIX)-gcc
AS=$(TOOLCHAIN_PREFIX)-as
CP=$(TOOLCHAIN_PREFIX)-objcopy
OD=$(TOOLCHAIN_PREFIX)-objdump

# Microcontroller family
DEFS = -DSTM32F1
# Cortex core
MCU = cortex-m3
MCFLAGS = -mcpu=$(MCU)


################## User Sources ####################
SRCS = src/main.c
SRCS += src/st7789_stm32_spi.c
SRCS += libopencm3/lib/cm3/vector.c
SRCS += src/usb_descriptor.c
SRCS += src/common.c

################## Includes ########################
INCLS = -I.
INCLS += -Ilibopencm3/include
INCLS += -Iinc
INCLS += -Ifonts
################## Libs ########################
LIBS = -Llibopencm3/lib
LIBS +=	-lopencm3_stm32f1

################ Compiler Flags ######################
CFLAGS = -ggdb
CFLAGS += -Wall -Wextra -Warray-bounds
CFLAGS += -mlittle-endian -mthumb -mthumb-interwork
CFLAGS += -mfloat-abi=soft
CFLAGS += --specs=nano.specs
CFLAGS += $(MCFLAGS)
	

############# CFLAGS for Optimization ##################
CFLAGS += -O0
CFLAGS += -ffunction-sections
CFLAGS += -fdata-sections


################### Linker Flags #####################
LFLAGS = -Tstm32f103xb.ld
LFLAGS += --specs=nosys.specs
LFLAGS += -Wl,--gc-sections -static
LFLAGS += -Wl,--Map=$(TARGET).map
LFLAGS += -Wl,--cref
LFLAGS += --static
LFLAGS += -nostartfiles #We're using no start file. Without this flag, get linker error


################### Recipe to make all (build and burn) ####################
.PHONY: all
all: build


################### Recipe to build ####################
.PHONY: build
build: $(TARGET).bin $(TARGET).hex $(TARGET).lst $(TARGET).size


################### Recipe to burn ####################
.PHONY: burn
burn:
	@echo "[Flashing] $(TARGET).bin in 0x8000000 address"
	@st-flash write $(TARGET).bin 0x8000000
	
	
################### Recipe to debug ####################
.PHONY: debug
debug:
	@echo "[Debugging] $(TARGET).elf"
	# Need to run st-util first for starting the gdb server
	@st-util &
	@$(GDB) -iex "add-auto-load-safe-path ./.gdbinit" $(TARGET).elf
	
	
################### Recipe to make .elf ####################
$(TARGET).elf: $(SRCS)
	@echo "[Compiling] $(SRCS)"
	@$(CC) $(INCLS) $(DEFS) $(CFLAGS) $(LFLAGS) $^ $(LIBS) -o $@
	
	
################### Recipe to make .bin ####################
$(TARGET).bin: $(TARGET).elf
	@echo "[Building] $@"
	@$(CP) -O binary $^ $@


################### Recipe to make .hex ####################
$(TARGET).hex: $(TARGET).elf
	@echo "[Building] $@"
	@$(CP) -O ihex $^ $@


################### Recipe to make .lst ####################
$(TARGET).lst: $(TARGET).elf
	@echo "[Building] $@"
	@$(OD) -S $^ > $(TARGET).lst


################### Recipe to make size ####################
$(TARGET).size: $(TARGET).elf
	@$(PREFIX)-size -B -d $^


################### Recipe to clean all ####################
.PHONY: clean
clean:
	@echo "[Cleaning] $(TARGET).hex $(TARGET).bin $(TARGET).elf $(TARGET).lst $(TARGET).map"
	@rm -f $(TARGET).hex $(TARGET).bin $(TARGET).elf $(TARGET).lst $(TARGET).map
	
	
