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

status_t set_lpuart_periferies_(void){

	/* enabling clock for port A(GPIOA), if it wasn't enable already */
	if(!(RCC->AHB2ENR & (1UL << RCC_AHB2ENR_GPIOAEN_Pos))){
		RCC->AHB2ENR|= (1UL << RCC_AHB2ENR_GPIOAEN_Pos);
	}

	/* enabling clock for LPUART1 periphery */
	RCC->APB1ENR2|=(1UL << RCC_APB1ENR2_LPUART1EN_Pos);

	/* configurating PA2, PA3 as alternate function pins */
	GPIOA->MODER&= ~((0x3UL << GPIO_MODER_MODE2_Pos)|(0x3UL << GPIO_MODER_MODE3_Pos));

	/* 0b10: Alternate function mode */
	GPIOA->MODER|=((0x2UL << GPIO_MODER_MODE2_Pos)|(0x2UL << GPIO_MODER_MODE3_Pos));
    /* LPUART1 for PA2,PA3 - AF12 1100 */
	GPIOA->AFR[0]&= ~((0xFUL << GPIO_AFRL_AFSEL2_Pos)|(0xFUL << GPIO_AFRL_AFSEL3_Pos));
	GPIOA->AFR[0]&= ~((12 << GPIO_AFRL_AFSEL2_Pos)|(12 << GPIO_AFRL_AFSEL3_Pos));

	return STATUS_OK;
}

status_t configure_lpuart1(void){
	/* disable uart before modify */
	LPUART1->CR1&= ~(1UL << USART_CR1_UE);
 /* 1 */
	/* set word length, M[1:0] = ‘00’: 1 Start bit, 8 Data bits, n Stop bit */
	LPUART1->CR1&= ~((1UL << USART_CR1_M0_Pos)|(1UL << USART_CR1_M1_Pos));

	/* 01: System clock (SYSCLK) selected as LPUART1 clock */
	RCC->CCIPR&= ~(3UL << RCC_CCIPR_LPUART1SEL_Pos);
	RCC->CCIPR|= (1UL << RCC_CCIPR_LPUART1SEL_Pos);

  /* 2 */
	/* so SYSCLK(configurated as 64 MHz is fLPUART_clk */
	/* BRR= (256×fLPUART_clk​​)/baudrate
	 * BRR = (256 * 64M)/115200
	 * BRR = 142 222(rounded) */
	LPUART1->BRR = 142222;
	/* enabling LPUART1 clock */
	RCC->APB1ENR2|=(1UL << RCC_APB1ENR2_LPUART1EN_Pos);

 /* 3 */
	/* 00: 1 stop bit */
	LPUART1->CR2&= ~(3UL << USART_CR2_STOP_Pos);

 /* 4 */
	/* enable uart before modify */
	LPUART1->CR1|=(1UL << USART_CR1_UE);

 /* 5 */
	/* disable DMA for receive */
	LPUART1->CR3&= ~(1UL << USART_CR3_DMAR_Pos);

	return STATUS_OK;


}

