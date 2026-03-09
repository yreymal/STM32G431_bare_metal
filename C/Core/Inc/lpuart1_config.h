/*
 * lpuart1_transmit_config.h
 *
 *  Created on: Feb 25, 2026
 *      Author: romanyarmak
 */

#ifndef INC_LPUART1_TRANSMIT_CONFIG_H_
#define INC_LPUART1_TRANSMIT_CONFIG_H_

#include <stm32g431xx.h>
#include "status.h"

#define SYSCLK_FREQ 64000000U
#define LPUART_BAUD 115200U

void lpuart1_init(void);
void lpuart1_send_byte(uint8_t sendByte);
void configure_lpuart_pins(void);
uint8_t lpuart1_receive_byte();

#endif /* INC_LPUART1_TRANSMIT_CONFIG_H_ */
