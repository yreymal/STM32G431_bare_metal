# CMake reads this file before project(). It prevents host compiler detection
# and configures the C++17 and assembly languages used by the firmware target.
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

find_program(ARM_GCC arm-none-eabi-gcc REQUIRED)
find_program(ARM_GXX arm-none-eabi-g++ REQUIRED)
find_program(ARM_OBJCOPY arm-none-eabi-objcopy REQUIRED)
find_program(ARM_SIZE arm-none-eabi-size REQUIRED)

set(CMAKE_CXX_COMPILER "${ARM_GXX}" CACHE FILEPATH "Arm C++ compiler" FORCE)
set(CMAKE_ASM_COMPILER "${ARM_GCC}" CACHE FILEPATH "Arm assembler driver" FORCE)
set(CMAKE_OBJCOPY "${ARM_OBJCOPY}" CACHE FILEPATH "Arm objcopy" FORCE)
set(CMAKE_SIZE "${ARM_SIZE}" CACHE FILEPATH "Arm size utility" FORCE)
