STM32G431 Bare-Metal Project
Build Instructions
==================

## Overview

This project can be built on **Cygwin** or  **MacOs** using either:

1. **CMake + Ninja** - generates only .elf file 
2. **Makefile (alternative method)** - generates .elf, .bin, .hex images for flashing the STM32 microcontroller

Both methods produce the firmware executable (`.elf`)

## Requirements

Ensure the following tools are installed:
* Make (if using the Makefile workflow)
For CMake:
* ARM GCC Toolchain (`arm-none-eabi-gcc`)
* CMake
* Ninja build system


Check installations:

make --version
arm-none-eabi-gcc --version
cmake --version
ninja --version

---

## Build Using Makefile

A Makefile is also provided for building the project without CMake.

Build the firmware:

**make all**

Clean generated files:

**make clean**

The Makefile build produces the same firmware outputs (`.elf`, `.bin`, `.hex`) depending on the Makefile configuration.

---

## Build Using CMake 

1. Configure the project using the preset:

cmake --preset stm32G431

2. Build the firmware:

cmake --build build

This will generate the firmware executable:

build/bin/stm32g431rb.elf

---

## Clean Build

To completely remove previous build artifacts:

**rm -rf build**

Then reconfigure and build again:

**cmake --preset stm32G431**
**cmake --build build**


---

## Notes

* The project targets the **STM32G431 microcontroller**.
* The build uses the **ARM GCC cross compiler (`arm-none-eabi-gcc`)**.
* The firmware is built for **bare-metal execution** without an operating system.
