
#ifndef SYS_TICK_H_
#define SYS_TICK_H_

#include <stm32g431xx.h>
#include "status.h"
#include "peripheral_configs.h"
/* reload = SysTick_clock / interrupt_frequency - 1 */

#define SYS_TICK_FREQ    (1000) /* 1ms */



/*
 * Initialises SysTick to generate an interrupt every 1 ms.
 *
 * hclk_hz:
 *   Frequency of the CPU/AHB clock used by SysTick. */ 

void SysTick_1_Init_ms(uint32_t hclk_hz);
/*
 * Returns the number of milliseconds since SysTick was started.
 *
 * This value is increased inside SysTick_Handler(). */
uint32_t get_ms(void);

#endif /* SYS_TICK_H_ */