set(CMAKE_SYSTEM_NAME Generic)   		# tell CMake target system is not the host OS, Generic means - No standard OS
set(CMAKE_SYSTEM_PROCESSOR cortex-m4)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/bin)

set(TOOLCHAIN_PATH C:/build_tool/T_gcc_arm~V7-2017-q4/T_gcc_arm/bin/arm-none-eabi-)
# Bare-metal toolchain: don't try to link a runnable exe during compiler checks
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(CMAKE_C_COMPILER   ${TOOLCHAIN_PATH}gcc.exe)
set(CMAKE_ASM_COMPILER ${TOOLCHAIN_PATH}gcc.exe)
set(CMAKE_OBJCOPY      ${TOOLCHAIN_PATH}objcopy.exe)
set(CMAKE_SIZE         ${TOOLCHAIN_PATH}size.exe)

# FLAGS description 
# -mcpu=cortex-m4        ARM Cortex-M4 core
# -mthumb 				Cortex-M processors only support Thumb-2 instruction set
# -mfpu=fpv4-sp-d16      From STM32G4 documentation: Single precision FPU, FPv4-SP-D16 --> FPv4, Single precision (SP), 16 registers
# -mfloat-abi=hard	    ‘hard’ allows generation of floating-point instructions and uses FPU-specific calling conventions.
set(MCU_FLAGS "-mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard")

 # enable warnings (Wextra enables additional compiler warnings)
 # NO optimization (for release will use -O2
 # max degug info
set(CMAKE_C_FLAGS "${MCU_FLAGS} -Wall -O0 -g3")

set(CMAKE_ASM_FLAGS "${MCU_FLAGS}")
