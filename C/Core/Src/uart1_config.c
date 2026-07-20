#include "uart1_config.h"

void configure_uart1_pins(){
   RCC->AHB2ENR|= (0x1UL << RCC_AHB2ENR_GPIOCEN_Pos);
   /* clean bits before set */
   GPIOC->MODER&= ~(GPIO_MODER_MODE4_Msk|GPIO_MODER_MODE5_Msk);
   /* 10: Alternate function mode */
   GPIOC->MODER|= ((0x2UL<<GPIO_MODER_MODE4_Pos) | (0x2UL<<GPIO_MODER_MODE5_Pos));

   /* 0: Output push-pull  */
   GPIOC->OTYPER&= ~(GPIO_OTYPER_OT4_Msk|GPIO_OTYPER_OT5_Msk);

    /* clean, set to 0 */
   GPIOC->OSPEEDR&= ~ (GPIO_OSPEEDR_OSPEED4_Msk | GPIO_OSPEEDR_OSPEED5_Msk);
    /* 10: High speed */
    GPIOC->OSPEEDR|= (2UL << GPIO_OSPEEDR_OSPEED4_Pos)|(2UL << GPIO_OSPEEDR_OSPEED5_Pos);
    /* clean, set to 0 */
    GPIOC->PUPDR&= ~ (GPIO_PUPDR_PUPD4_Msk | GPIO_PUPDR_PUPD5_Msk);
    /* for TX is no pull up/down,
     * for RX set pull up to reduce noise, not driven by MCU
     */
    /* 01: Pull-up for Rx*/
    GPIOC->PUPDR|=  (1UL << GPIO_PUPDR_PUPD5_Pos);
    
    /* USART1_ Tx/Rx - AF7
     * 0111: AF7
     */
    GPIOC->AFR[0]&= ~(GPIO_AFRL_AFSEL4_Msk|GPIO_AFRL_AFSEL5_Msk);
    GPIOC->AFR[0]|= ((7UL << GPIO_AFRL_AFSEL4_Pos) | (7UL << GPIO_AFRL_AFSEL5_Pos));

}


/*

Character transmission procedure
To transmit a character, follow the sequence below:
1. Program the M bits in USART_CR1 to define the word length.
2. Select the desired baud rate using the USART_BRR register.
3. Program the number of stop bits in USART_CR2.
4. Enable the USART by writing the UE bit in USART_CR1 register to 1.
5. Select DMA enable (DMAT) in USART_CR3 if multibuffer communication must take
place. Configure the DMA register as explained in Section 37.5.19: Continuous
communication using USART and DMA.
6. Set the TE bit in USART_CR1 to send an idle frame as first transmission.
7. Write the data to send in the USART_TDR register. Repeat this for each data to be
transmitted in case of single buffer.
– When FIFO mode is disabled, writing a data to the USART_TDR clears the TXE
flag.
– When FIFO mode is enabled, writing a data to the USART_TDR adds one data to
the TXFIFO. Write operations to the USART_TDR are performed when TXFNF
flag is set. This flag remains set until the TXFIFO is full.
8. When the last data is written to the USART_TDR register, wait until TC = 1.
– When FIFO mode is disabled, this indicates that the transmission of the last frame
is complete.
– When FIFO mode is enabled, this indicates that both TXFIFO and shift register are
empty.
This check is required to avoid corrupting the last transmission when the USART is
disabled or enters Halt mode.

*/

void init_uart1(){

 /* enabling clock for USART1 periphery in APB2 bus */
 RCC->APB2ENR|= RCC_APB2ENR_USART1EN_Msk;

 /* ‘00’: 1 start bit, 8 Data bits, n Stop bit */
USART1->CR1 &= ~(USART_CR1_M0_Msk | USART_CR1_M1_Msk);

 /* 01: System clock (SYSCLK) selected as USART1 clock */
RCC->CCIPR &= ~ RCC_CCIPR_USART1SEL_Msk;
RCC->CCIPR|= (1UL << RCC_CCIPR_USART1SEL_Pos);

/* 0000: input clock not divided */
USART1->PRESC &= ~ USART_PRESC_PRESCALER_Msk;

/* clock sourse SYSCLK(64MHz), prescaler = 1
 * BRR = 64MHz/BAUD = 64MHz/9600 = 6666.6
 */
USART1->BRR &= ~ USART_BRR_BRR_Msk;
USART1->BRR = 6667;
/* 00: 1 stop bit */
USART1->CR2 &= ~ USART_CR2_STOP_Msk;

/* 1: USART enabled*/
USART1->CR1|=(1UL << USART_CR1_UE_Pos);
/* 1: Transmitter is enabled */
USART1->CR1|=(1UL << USART_CR1_TE_Pos);
}

void send_uart1(uint8_t byte){
    /* wait intil Transmit Data Register is Empty */
  while(!(USART1->ISR & USART_ISR_TXE_Msk)){}
  USART1->TDR = byte;
}

uint8_t receive_uart1(){

}