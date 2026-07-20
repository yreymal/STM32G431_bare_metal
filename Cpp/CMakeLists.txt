cmake_minimum_required(VERSION 3.22)

# The application and CMSIS system layer are C++17. ASM is retained for the
# vendor startup/vector-table file.
project(stm32g431rb LANGUAGES CXX ASM)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_HOST_TESTS "Build hardware-independent tests for the PC" ON)
option(BUILD_STM32_FIRMWARE "Build the complete STM32G431RB ELF" OFF)

# A cross-compiled test executable cannot run on the host, so the two modes are
# deliberately mutually exclusive.
if(BUILD_HOST_TESTS AND BUILD_STM32_FIRMWARE)
    message(FATAL_ERROR
        "Choose one mode: BUILD_HOST_TESTS or BUILD_STM32_FIRMWARE")
endif()

set(PROJECT_INC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Core/Inc")
set(PROJECT_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Core/Src")

set(PROJECT_WARNING_OPTIONS
    -Wall
    -Wextra
    -Wpedantic
    -Wconversion
    -Wsign-conversion
    -Wshadow
    -Wundef
    -Wformat=2
    -Wdouble-promotion
    -Werror)

# -----------------------------------------------------------------------------
# Host tests
# -----------------------------------------------------------------------------
# This target contains only hardware-independent code and is built by the
# host-debug preset with the normal desktop C++ compiler.
if(BUILD_HOST_TESTS)
    enable_testing()

    add_library(project_logic STATIC
        "${PROJECT_SRC_DIR}/scheduler.cpp")
    target_include_directories(project_logic PUBLIC "${PROJECT_INC_DIR}")
    target_compile_options(project_logic PRIVATE ${PROJECT_WARNING_OPTIONS})

    add_executable(unit_tests tests/unit_tests.cpp)
    target_link_libraries(unit_tests PRIVATE project_logic)
    target_compile_options(unit_tests PRIVATE ${PROJECT_WARNING_OPTIONS})
    add_test(NAME project_unit_tests COMMAND unit_tests)
endif()

# -----------------------------------------------------------------------------
# STM32 firmware
# -----------------------------------------------------------------------------
# The STM32 presets select cmake/arm-none-eabi-toolchain.cmake before CMake runs
# project(), ensuring that the C++ linker driver is arm-none-eabi-g++.
if(BUILD_STM32_FIRMWARE)
    # CACHE variables allow a different platform layout to be supplied with
    # -DNAME=/another/path without editing this file.
    set(STM32_STARTUP_FILE
        "${CMAKE_CURRENT_SOURCE_DIR}/Core/Startup/startup_stm32g431rbtx.s"
        CACHE FILEPATH "STM32 startup assembly file")
    set(STM32_LINKER_SCRIPT
        "${CMAKE_CURRENT_SOURCE_DIR}/linker/STM32G431RBTX_FLASH.ld"
        CACHE FILEPATH "STM32G431RB Flash linker script")
    set(STM32_DRIVERS_DIR
        "${CMAKE_CURRENT_SOURCE_DIR}/Core/Drivers"
        CACHE PATH "STM32 C++ driver integration directory")

    # Stop during CMake configuration with a useful message instead of failing
    # much later in the compiler or linker with a confusing missing-file error.
    set(STM32_REQUIRED_PATHS
        "${STM32_STARTUP_FILE}"
        "${STM32_LINKER_SCRIPT}"
        "${STM32_DRIVERS_DIR}/CMakeLists.txt"
        "${STM32_DRIVERS_DIR}/CMSIS/Device/ST/STM32G4xx/Include"
        "${STM32_DRIVERS_DIR}/CMSIS/Include")
    foreach(required_path IN LISTS STM32_REQUIRED_PATHS)
        if(NOT EXISTS "${required_path}")
            message(FATAL_ERROR
                "Required STM32 platform path is missing: ${required_path}")
        endif()
    endforeach()

    add_subdirectory("${STM32_DRIVERS_DIR}"
        "${CMAKE_CURRENT_BINARY_DIR}/stm32-drivers")

    # Application modules replacing the old .c configuration files. Keep this
    # list explicit so a release build cannot silently acquire an unrelated file.
    set(APPLICATION_SOURCES
        "${PROJECT_SRC_DIR}/board.cpp"
        "${PROJECT_SRC_DIR}/interrupt_handlers.cpp"
        "${PROJECT_SRC_DIR}/interrupts.cpp"
        "${PROJECT_SRC_DIR}/lpuart1.cpp"
        "${PROJECT_SRC_DIR}/main.cpp"
        "${PROJECT_SRC_DIR}/safety_manager.cpp"
        "${PROJECT_SRC_DIR}/scheduler.cpp"
        "${PROJECT_SRC_DIR}/seven_segment_display.cpp"
        "${PROJECT_SRC_DIR}/sys_tick.cpp"
        "${PROJECT_SRC_DIR}/timer6.cpp"
        "${PROJECT_SRC_DIR}/timer8.cpp"
        "${PROJECT_SRC_DIR}/uart1.cpp")

    # Only the startup/vector table remains in assembly. Do not also add
    # old main.c, stm32g4xx_it.c, scheduler.c, or peripheral configuration .c
    # files: they would duplicate the symbols implemented by the C++ modules.
    add_executable(${PROJECT_NAME}
        ${APPLICATION_SOURCES}
        "${STM32_STARTUP_FILE}")

    target_include_directories(${PROJECT_NAME} PRIVATE "${PROJECT_INC_DIR}")
    target_link_libraries(${PROJECT_NAME} PRIVATE STM32::CMSIS)

    set(MCU_OPTIONS
        -mcpu=cortex-m4
        -mthumb
        -mfpu=fpv4-sp-d16
        -mfloat-abi=hard)

    target_compile_options(${PROJECT_NAME} PRIVATE
        ${MCU_OPTIONS}
        $<$<COMPILE_LANGUAGE:CXX>:-ffunction-sections>
        $<$<COMPILE_LANGUAGE:CXX>:-fdata-sections>
        $<$<COMPILE_LANGUAGE:CXX>:-fstack-usage>

        # Strict project policy applies to every C++ application module.
        $<$<COMPILE_LANGUAGE:CXX>:-Wall>
        $<$<COMPILE_LANGUAGE:CXX>:-Wextra>
        $<$<COMPILE_LANGUAGE:CXX>:-Wpedantic>
        $<$<COMPILE_LANGUAGE:CXX>:-Wconversion>
        $<$<COMPILE_LANGUAGE:CXX>:-Wsign-conversion>
        $<$<COMPILE_LANGUAGE:CXX>:-Wshadow>
        $<$<COMPILE_LANGUAGE:CXX>:-Wundef>
        $<$<COMPILE_LANGUAGE:CXX>:-Wformat=2>
        $<$<COMPILE_LANGUAGE:CXX>:-Wdouble-promotion>
        $<$<COMPILE_LANGUAGE:CXX>:-Werror>

        # Deterministic embedded C++ runtime subset.
        $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>
        $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
        $<$<COMPILE_LANGUAGE:CXX>:-fno-threadsafe-statics>
        $<$<COMPILE_LANGUAGE:CXX>:-fno-use-cxa-atexit>)

    target_link_options(${PROJECT_NAME} PRIVATE
        ${MCU_OPTIONS}
        -T${STM32_LINKER_SCRIPT}
        -static
        --specs=nano.specs
        --specs=nosys.specs
        -Wl,--gc-sections
        -Wl,--cref
        -Wl,--print-memory-usage
        -Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.map)

    # The C++ linker driver adds its runtime automatically. These are the
    # embedded C, math, and no-host-system libraries used by newlib.
    target_link_libraries(${PROJECT_NAME} PRIVATE c m nosys)

    set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".elf")

    # Produce common flashing formats and print the final section sizes.
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O ihex
                $<TARGET_FILE:${PROJECT_NAME}>
                ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.hex
        COMMAND ${CMAKE_OBJCOPY} -O binary
                $<TARGET_FILE:${PROJECT_NAME}>
                ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.bin
        COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${PROJECT_NAME}>
        COMMENT "Generating HEX/BIN and reporting firmware size")
endif()
