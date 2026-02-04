/*
 * interruptConfig.h
 *
 *  Created on: Feb 1, 2026
 *      Author: romanyarmak
 */
#include "status.h"
#include <stdint.h>
#include <stm32g431xx.h>
#ifndef INC_INTERRUPTCONFIG_H_
#define INC_INTERRUPTCONFIG_H_

status_t configureInterrupt(uint32_t gpioPort, uint8_t pinNumber);


#endif /* INC_INTERRUPTCONFIG_H_ */
