/*
 * lpuart1_receive_config.c
 *
 *  Created on: Feb 25, 2026
 *      Author: romanyarmak
 */

#include "lpuart1_receive_config.h"

/*
 * Character reception procedure
To receive a character, follow the sequence below:
1. Program the M bits in LPUART_CR1 to define the word length.
2. Select the desired baud rate using the baud rate register LPUART_BRR
3. Program the number of stop bits in LPUART_CR2.
4. Enable the LPUART by writing the UE bit in LPUART_CR1 register to ‘1’.
5. Select DMA enable (DMAR) in LPUART_CR3 if multibuffer communication is to take
place. Configure the DMA register as explained in Section 38.4.12: Continuous
communication using DMA and LPUART.
6. Set the RE bit LPUART_CR1. This enables the receiver which begins searching for a
start bit.
 * */
