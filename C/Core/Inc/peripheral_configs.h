/*
 * peripheral_configs.h
 *
 *  Created on: Jan 31, 2026
 *      Author: romanyarmak
 */

#ifndef INC_PERIPHERAL_CONFIGS_H_
#define INC_PERIPHERAL_CONFIGS_H_

#include <stdint.h>
#include <stm32g431xx.h>
#include "status.h"
#include "reg_bits_check.h"

/**
  * @brief  The application entry point.
  * @retval int
  */

int configureUserButton();

status_t ConfigureClock();

status_t initGPIOA5();

int delay(const uint32_t timer);

status_t configure_MCO_pinA8();







#endif /* INC_PERIPHERAL_CONFIGS_H_ */
