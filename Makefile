#	23/02/2026 Yarmak
# ------------------------------------------------

######################################
# target
######################################
TARGET = STM32G431_bare_metal-main


######################################
# building variables
######################################
# debug build?
DEBUG = 1
# optimization
OPT = -Og


#######################################
# paths
#######################################
# Build path
BUILD_DIR = build

######################################

# Includes
# -----------------------------
C_INCLUDES := \
-ICore/Inc \
-IDrivers/STM32G4xx_HAL_Driver/Inc \
-IDrivers/STM32G4xx_HAL_Driver/Inc/Legacy \
-IDrivers/CMSIS/Device/ST/STM32G4xx/Include \
-IDrivers/CMSIS/Include

# -----------------------------
# Sources
# -----------------------------
# Core
C_SOURCES := \
Core/Src/interruptConfig.c \
Core/Src/main.c \
Core/Src/lpuart1_transmit_config.c \
Core/Src/peripheral_configs.c \
Core/Src/seven_segment.c \
Core/Src/stm32g4xx_it.c \
Core/Src/syscalls.c \
Core/Src/sysmem.c \
Core/Src/system_stm32g4xx.c

# HAL
C_SOURCES += \
Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal.c \
Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_cortex.c \
Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_dma.c \
Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_dma_ex.c \
Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_exti.c \
Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_flash.c \
Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_flash_ex.c \
Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_flash_ramfunc.c \
Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_gpio.c \
Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_pwr.c \
Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_pwr_ex.c \
Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_rcc.c \
Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_rcc_ex.c

# Startup 
ASM_SOURCES := Core/Startup/startup_stm32g431rbtx.s

#######################################
# binaries
#######################################
OS = $(shell uname -s)
PREFIX    := arm-none-eabi-
ifeq (Darwin, $(OS))
TOOLCHAIN ?= /Applications/ArmGNUToolchain/12.3.rel1/arm-none-eabi/bin
CC  := $(PREFIX)gcc
AS  := $(PREFIX)gcc 
CP  := $(PREFIX)objcopy
SZ  := $(PREFIX)size
###############################
else ifneq (,$(findstring CYGWIN,$(OS)))
#Windows(Cygwin)
TOOLCHAIN = C:/build_tool/T_gcc_arm~V7-2017-q4/T_gcc_arm/bin
CC := $(TOOLCHAIN)/$(PREFIX)gcc.exe
AS := $(TOOLCHAIN)/$(PREFIX)gcc.exe 
CP := $(TOOLCHAIN)/$(PREFIX)objcopy.exe
SZ := $(TOOLCHAIN)/$(PREFIX)size.exe
else
    $(error Unsupported platform: $(OS))
endif


HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S

###############################
# ---- DEBUG PRINTS (temporary) ----

#$(info SZ=$(SZ))
#$(info SIZE=$(SIZE))
#$(info OBJCOPY=$(OBJCOPY))
#$(info CP=$(CP))
#$(info HEX=$(HEX))
#$(info BIN=$(BIN))
# ---------------------------------
#######################################
# CFLAGS
#######################################
# CPU
CPU = -mcpu=cortex-m4

# FPU (STM32G4 has hardware FPU)
FPU = -mfpu=fpv4-sp-d16

# Floating point ABI
FLOAT-ABI = -mfloat-abi=hard

# MCU
MCU = $(CPU) -mthumb $(FPU) $(FLOAT-ABI)

# macros for gcc
# AS defines
AS_DEFS = 

# C defines
C_DEFS =  \
-DUSE_HAL_DRIVER \
-DSTM32G431xx



# compile gcc flags
ASFLAGS = $(MCU) $(AS_DEFS) $(AS_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

CFLAGS += $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

ifeq ($(DEBUG), 1)
CFLAGS += -g -gdwarf-2
endif


# Generate dependency information
CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)"


#######################################
# LDFLAGS
#######################################
# link script
LDSCRIPT = STM32G431RBTX_FLASH.ld

# libraries
LIBS = -lc -lm -lnosys 
LIBDIR = 
LDFLAGS = $(MCU) -specs=nano.specs -T$(LDSCRIPT) $(LIBDIR) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections

# default action: build all
########        ALL    ###############
all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin
######## 	ALL	##############

#######################################
# build the application
#######################################
# list of objects
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))
# list of ASM program objects
OBJECTS = $(patsubst %.c,$(BUILD_DIR)/%.o,$(C_SOURCES)) \
          $(patsubst %.s,$(BUILD_DIR)/%.o,$(ASM_SOURCES))

$(BUILD_DIR)/%.o: %.c Makefile
	@mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/%.o: %.s Makefile
	@mkdir -p $(dir $@)
	$(AS) -c $(ASFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) Makefile
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	$(SZ) $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(HEX) $< $@

$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(BIN) $< $@

$(BUILD_DIR):
	mkdir $@		

#######################################
# clean up
#######################################
clean:
	-rm -fR $(BUILD_DIR)
  
#######################################
# dependencies
#######################################
-include $(wildcard $(BUILD_DIR)/*.d)

###########################################
#Flashing commands for ST-Link or OpenOCD #
###########################################

JLINK = "C:/opt_tool/J-Link v7.98i/JLink_V798i/JLink.exe"
DEVICE = STM32F103C8
INTERFACE = SWD
SPEED = 4000
BIN_FILE = $(BUILD_DIR)/$(TARGET).bin
LOAD_ADDR = 0x08000000

jflash: $(BIN_FILE)
    $(JLINK) -CommandFile jlink-flash.jlink

flash: $(BUILD_DIR)/$(TARGET).bin
	$(STL) write $(BUILD_DIR)/$(TARGET).bin 0x8000000

erase:
	$(STL) erase

openocd-flash: $(BUILD_DIR)/$(TARGET).bin
	openocd -f interface/ftdi/ft232h-module-swd.cfg -f target/stm32g4x.cfg \
	-c "init" -c "reset halt" \
	-c "flash write_image erase $(BUILD_DIR)/$(TARGET).bin 0x08000000 bin" \
	-c "reset run" -c "shutdown"
	
openocd-erase:
	openocd -f interface/ftdi/ft232h-module-swd.cfg -f target/stm32g4x.cfg \
	-c "init" -c "reset halt" \
	-c "stm32g4x mass_erase 0" \
	-c "reset run" -c "shutdown"