/*
 * tim6_config.h
 *
 *  Created on: Jun 9, 2026
 *      Author: romanyarmak
 */

#ifndef TIM6_CONFIG_H_
#define TIM6_CONFIG_H_

#include <stm32g431xx.h>
#include "status.h"

status_t tim6_init();
void toggle_tim6(void);
uint8_t get_seconds_tim6();
#endif /* TIM6_CONFIG_H_ */