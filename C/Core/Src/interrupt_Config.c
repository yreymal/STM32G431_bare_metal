/*
 * interruptConfig.c
 *
 *  Created on: Feb 1, 2026
 *      Author: romanyarmak
 */

#include "interrupt_config.h"
#include "seven_segment.h"

status_t exti_configure_pa0_pa1(){

	/* if GPIOA wasn't enable before, enable */

	if(!(RCC->AHB2ENR & (0x1UL<<RCC_AHB2ENR_GPIOAEN_Pos))){
		RCC->AHB2ENR|= (0x1UL<<RCC_AHB2ENR_GPIOAEN_Pos);
	}

	/* we need activate syscfg to be able choose GPIO port for each EXTI interrupt line */
     if (!(RCC->APB2ENR & 0x1UL<<RCC_APB2ENR_SYSCFGEN_Pos)){
    	 RCC->APB2ENR|=(0x1UL<<RCC_APB2ENR_SYSCFGEN_Pos);
     }

     /* configure pins A0 A1 as input with pull-up registers,
      *  because we use buttons connected to GRF */

     /* 00: Input mode  */
     GPIOA->MODER&=~((0x3UL<<GPIO_MODER_MODE0_Pos)|(0x3UL<<GPIO_MODER_MODE1_Pos));


     /* clear bits */
     GPIOA->PUPDR&=~((0x3UL<<GPIO_PUPDR_PUPD0_Pos)|(0x3UL<<GPIO_PUPDR_PUPD1_Pos));
     /* PUPDR 01: Pull-up */
     GPIOA->PUPDR|=((0x1UL<<GPIO_PUPDR_PUPD0_Pos)|(0x1UL<<GPIO_PUPDR_PUPD1_Pos));

     /* route pins with EXTIs */
     /* 0000: PA[0] pin */
     SYSCFG->EXTICR[0]&=~(0b1111<<SYSCFG_EXTICR1_EXTI0_Pos);
     /* 0000: PA[1] pin */
     SYSCFG->EXTICR[0]&=~(0b1111<<SYSCFG_EXTICR1_EXTI1_Pos);

     /* buttons are connected to GND, so we use pull-up to VCC*
      * => trigger by falling enge, rising interrupt disable */

     /* rising triggers disabling
      * 0: Rising trigger disabled (for Event and Interrupt) for input line */
     EXTI->RTSR1&=~((1UL << EXTI_RTSR1_RT0_Pos)|(1UL << EXTI_RTSR1_RT1_Pos));

     /* falling triggers enabling *
      * 1: Falling trigger enabled (for Event and Interrupt) for input line
      */
     EXTI->FTSR1|=((1UL << EXTI_FTSR1_FT0_Pos)|(1UL << EXTI_FTSR1_FT1_Pos));

     /* unmask(allow) interrupts for 0 and 1 pins
      * 1: Interrupt request from Line x is not masked*/
     EXTI->IMR1|=((1UL << EXTI_IMR1_IM0_Pos)|(1UL << EXTI_IMR1_IM1_Pos));

     /* set priority with CMSIS function */
     /* interrupt pin 0 with priority 6 */
     NVIC_SetPriority(EXTI0_IRQn,6);
     /* interrupt pin 1 with priority 7 */
     NVIC_SetPriority(EXTI1_IRQn,7);

     /* Enable interrupts */
     NVIC_EnableIRQ(EXTI0_IRQn);
     NVIC_EnableIRQ(EXTI1_IRQn);

     return STATUS_OK;

}

void EXTI0_IRQHandler(){
 if(EXTI->PR1 & (1<<EXTI_PR1_PIF0_Pos)){

	 GPIOA->ODR =  ((GPIOA->ODR & (~SEG_7_ALL_MSK)) | segment_numbers[8]);

     /* That is very bad practice in embedded to use delays in an interrupt*/
     /* that usage serves only for prototyping! */
	 for(uint32_t j = 0;j<5000000;++j);

	 EXTI->PR1 = (1UL << EXTI_PR1_PIF0_Pos);
 }


}

void EXTI1_IRQHandler(){
	if(EXTI->PR1 & (1<<EXTI_PR1_PIF1_Pos)){
		 GPIOA->ODR =  segment_numbers[8];
	  
         /* That is very bad practice in embedded to use delays in an interrupt*/
         /* that usage serves only for prototyping! */
		 for(uint32_t j = 0;j<5000000;++j);

	   EXTI->PR1 = (1UL << EXTI_PR1_PIF1_Pos);
	 }

}
