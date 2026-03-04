
cmake_minimum_required(VERSION 3.22)

project(stm32g431rb C ASM)

set(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/STM32G431RBTX_FLASH.ld)

add_executable(${PROJECT_NAME}
	${CMAKE_SOURCE_DIR}/Core/Startup/startup_stm32g431rbtx.s
	${CMAKE_SOURCE_DIR}/Core/Src/interruptConfig.c 
	${CMAKE_SOURCE_DIR}/Core/Src/main.c 
	${CMAKE_SOURCE_DIR}/Core/Src/lpuart1_transmit_config.c 
	${CMAKE_SOURCE_DIR}/Core/Src/peripheral_configs.c 
	${CMAKE_SOURCE_DIR}/Core/Src/seven_segment.c 
	${CMAKE_SOURCE_DIR}/Core/Src/stm32g4xx_it.c 
	${CMAKE_SOURCE_DIR}/Core/Src/syscalls.c 
	${CMAKE_SOURCE_DIR}/Core/Src/sysmem.c 
	${CMAKE_SOURCE_DIR}/Core/Src/system_stm32g4xx.c
	${CMAKE_SOURCE_DIR}/Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal.c 
	${CMAKE_SOURCE_DIR}/Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_cortex.c 
	${CMAKE_SOURCE_DIR}/Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_dma.c 
	${CMAKE_SOURCE_DIR}/Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_dma_ex.c 
	${CMAKE_SOURCE_DIR}/Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_exti.c 
	${CMAKE_SOURCE_DIR}/Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_flash.c 
	${CMAKE_SOURCE_DIR}/Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_flash_ex.c 
	${CMAKE_SOURCE_DIR}/Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_flash_ramfunc.c 
	${CMAKE_SOURCE_DIR}/Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_gpio.c 
	${CMAKE_SOURCE_DIR}/Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_pwr.c 
	${CMAKE_SOURCE_DIR}/Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_pwr_ex.c 
	${CMAKE_SOURCE_DIR}/Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_rcc.c 
	${CMAKE_SOURCE_DIR}/Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_rcc_ex.c
	
)

target_compile_definitions(${PROJECT_NAME} PRIVATE STM32G431xx)

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_SOURCE_DIR}/Core/Inc
	${CMAKE_SOURCE_DIR}/Drivers/STM32G4xx_HAL_Driver/Inc 
	${CMAKE_SOURCE_DIR}/Drivers/STM32G4xx_HAL_Driver/Inc/Legacy 
	${CMAKE_SOURCE_DIR}/Drivers/CMSIS/Device/ST/STM32G4xx/Include
	${CMAKE_SOURCE_DIR}/Drivers/CMSIS/Include
)

target_link_options(${PROJECT_NAME} PRIVATE
    -T${LINKER_SCRIPT}
    -Wl,-Map=${PROJECT_NAME}.map
    -Wl,--gc-sections
)

target_link_libraries(${PROJECT_NAME}
    -lc
    -lm
    -lnosys
)

set_target_properties(${PROJECT_NAME} PROPERTIES
  SUFFIX ".elf"
)
