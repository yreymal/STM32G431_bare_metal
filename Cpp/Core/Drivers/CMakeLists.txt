add_library(stm32g4_cmsis INTERFACE)
add_library(STM32::CMSIS ALIAS stm32g4_cmsis)

# CMSIS device headers are supplied by ST as C-compatible headers, but they
# also provide the required extern "C" guards and compile cleanly as C++17.
# Keep the vendor files unchanged and expose them as a system dependency so
# project warning policy remains focused on code owned by this repository.
target_include_directories(stm32g4_cmsis SYSTEM INTERFACE
    "${CMAKE_CURRENT_LIST_DIR}/CMSIS/Device/ST/STM32G4xx/Include"
    "${CMAKE_CURRENT_LIST_DIR}/CMSIS/Include")

target_compile_definitions(stm32g4_cmsis INTERFACE STM32G431xx)
target_compile_features(stm32g4_cmsis INTERFACE cxx_std_17)

# This source provides the CMSIS system symbols used by the startup file. It is
# propagated to the firmware executable and therefore receives the executable's
# Cortex-M4 compile options and warning policy.
target_sources(stm32g4_cmsis INTERFACE
    "${CMAKE_CURRENT_LIST_DIR}/Src/cmsis_device.cpp")
