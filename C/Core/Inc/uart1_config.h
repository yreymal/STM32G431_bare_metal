/*
 * uart1_config.h
 *
 *  Created on: Jun 11, 2026
 *      Author: romanyarmak
 */



#ifndef UART1_CONFIG_H
#define UART1_CONFIG_H

#include <stm32g431xx.h>

void configure_uart1_pins();
void init_uart1();
void send_uart1(uint8_t byte);
uint8_t receive_uart1();

#endif /* UART1_CONFIG_H */