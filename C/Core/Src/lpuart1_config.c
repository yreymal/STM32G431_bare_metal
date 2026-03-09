/*
 * lpuart1_config.c
 *
 *  Created on: Feb 25, 2026
 *      Author: romanyarmak
 */

#include "lpuart1_config.h"



/*
 * From Reference Manual: 
 *
 * Character transmission procedure
To transmit a character, follow the sequence below:
1. Program the M bits in LPUART_CR1 to define the word length.
2. Select the desired baud rate using the LPUART_BRR register.
3. Program the number of stop bits in LPUART_CR2.
4. Enable the LPUART by writing the UE bit in LPUART_CR1 register to ‘1’.
5. Select DMA enable (DMAT) in LPUART_CR3 if Multi buffer Communication is to take
place. Configure the DMA register as explained in Section 38.4.12: Continuous
communication using DMA and LPUART.
6. Set the TE bit in LPUART_CR1 to send an idle frame as first transmission.
7. Write the data to send in the LPUART_TDR register. Repeat this operation for each
data to be transmitted in case of single buffer.
– When FIFO mode is disabled, writing a data in the LPUART_TDR clears the TXE
flag.
– When FIFO mode is enabled, writing a data in the LPUART_TDR adds one data to
the TXFIFO. Write operations to the LPUART_TDR are performed when TXFNF flag
is set. This flag remains set until the TXFIFO is full.
8. When the last data is written to the LPUART_TDR register, wait until TC = 1. This
indicates that the transmission of the last frame is complete.
– When FIFO mode is disabled, this indicates that the transmission of the last frame is
complete.
 */

void configure_lpuart_pins(void){

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
	GPIOA->AFR[0]|= ((12 << GPIO_AFRL_AFSEL2_Pos)|(12 << GPIO_AFRL_AFSEL3_Pos));

}

void lpuart1_init(void){

	/* turning off lpuart before configurate it */
	/* UE: LPUART disable */
	/* 0: LPUART prescaler and outputs disabled, low-power mode */
	LPUART1->CR1&=~ (1UL << USART_CR1_UE_Pos);

/* 1 */
	/* M1[1:0] = ‘00’: 1 Start bit, 8 Data bits, n Stop bit
	 * M0 - legacy from old STM F series*/
	LPUART1->CR1&=~ ((1UL << USART_CR1_M0_Pos)|(1UL << USART_CR1_M1_Pos));

	/* clear 2 LPUART bits for RCC clock selection*/
	RCC->CCIPR&=~ (3UL << RCC_CCIPR_LPUART1SEL_Pos);

	/* 01: System clock (SYSCLK) selected as LPUART1 clock */
	RCC->CCIPR|=(1UL << RCC_CCIPR_LPUART1SEL_Pos);

/* 2 */
	/* so SYSCLK(configurated as 64 MHz is fLPUART_clk */
	/* BRR= (256×fLPUART_clk​​)/baudrate
	 * BRR = (256 * 64M)/115200
	 * BRR = 142 222(rounded) */
	LPUART1->BRR = (256U * SYSCLK_FREQ) / LPUART_BAUD;
/* 3 */
	/* 00: 1 stop bit */
	LPUART1->CR2&= ~(3UL << USART_CR2_STOP_Pos);
/* 4 */
	/* UE: LPUART enable */
	LPUART1->CR1|=(1UL << USART_CR1_UE_Pos);
/* 5 */
	/* 0: DMA mode is disabled for transmission */
	LPUART1->CR3&=~(1UL << USART_CR3_DMAT_Pos);

	/* 0: DMA mode is disabled for reception */
	LPUART1->CR3&=~(1UL << USART_CR3_DMAR_Pos);
/* 6 */
	/* Enable the transmitter. When you do,
	 * the TX line goes to the idle (HIGH) state before any data is sent..*/
	LPUART1->CR1|=(1UL << USART_CR1_TE_Pos);

	/* enable LPUART for receiving*/
	LPUART1->CR1|=(1UL << USART_CR1_RE_Pos);
}

void lpuart1_send_byte(uint8_t sendByte){

	/* waiting while we can send byte*
	 * FIFO mode disabled
	 * USART interrupt and status register [alternate] (USART_ISR)*
	 */

	/* 1: Data register not full */
	while(!(LPUART1->ISR & (1UL << USART_ISR_TXE_Pos)));
	LPUART1->TDR = sendByte;

}

uint8_t lpuart1_receive_byte(){
	   while (!(LPUART1->ISR & USART_ISR_RXNE_RXFNE)) {}
	    return (uint8_t)LPUART1->RDR;

}