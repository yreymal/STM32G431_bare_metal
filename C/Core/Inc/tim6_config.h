/*
 * tim6_config.h
 *
 *  Created on: Jun 9, 2026
 *      Author: romanyarmak
 */

#ifndef TIM6_CONFIG_H_
#define TIM6_CONFIG_H_

#include <stm32g431xx.h>
#include "dynamic_4dig_7seg.h"
#include "status.h"
#include "peripheral_configs.h"

/* tclk is equal sys clk = 100MHz /
 * update_frequency = TIM6_CLK / ((PSC + 1) * (ARR + 1)
*/

#define PSC_DEF 10000
#define ARR_DEF 10000


status_t tim6_init();
void toggle_tim6(void);
uint8_t get_seconds_tim6();
void store_seconds_tim6(void *par);
uint8_t get_digit();
#endif /* TIM6_CONFIG_H_ */