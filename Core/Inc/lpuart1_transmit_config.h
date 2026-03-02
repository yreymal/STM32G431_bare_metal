/*
 * lpuart1_transmit_config.h
 *
 *  Created on: Feb 25, 2026
 *      Author: romanyarmak
 */

#ifndef INC_LPUART1_TRANSMIT_CONFIG_H_
#define INC_LPUART1_TRANSMIT_CONFIG_H_

#include <stm32g431xx.h>

void configure_lpuart_transmit(void);
void sendLPUART1(uint8_t b);
uint8_t receiveLPUART1();

#endif /* INC_LPUART1_TRANSMIT_CONFIG_H_ */
