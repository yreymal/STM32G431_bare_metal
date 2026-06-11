/*
 * seven_segment.c
 *
 *  Created on: Jan 31, 2026
 *      Author: romanyarmak
 */

#include "seven_segment.h"

/*PA4-PA10
 * OUTPUT - configuration must be push-pull */
status_t configure_7_seg_pins(void){

	/* check whether GPIOA clock is already enabled, if not, turn it on */
	if(!(RCC->AHB2ENR & (1UL<<RCC_AHB2ENR_GPIOAEN_Pos))){
		RCC->AHB2ENR|=(1UL<<RCC_AHB2ENR_GPIOAEN_Pos);
	}

	/* MODER has 2 bits fields, set 0 to all pins from PA4 till PA10 */
	GPIOA->MODER&=~((0b11<<GPIO_MODER_MODE4_Pos)
			       |(0b11<<GPIO_MODER_MODE5_Pos)
				   |(0b11<<GPIO_MODER_MODE6_Pos)
				   |(0b11<<GPIO_MODER_MODE7_Pos)
				   |(0b11<<GPIO_MODER_MODE11_Pos)
				   |(0b11<<GPIO_MODER_MODE9_Pos)
				   |(0b11<<GPIO_MODER_MODE10_Pos)
			);
	/* 01: General purpose output mode */
	GPIOA->MODER|=((1UL<<GPIO_MODER_MODE4_Pos)
				  |(1UL<<GPIO_MODER_MODE5_Pos)
				  |(1UL<<GPIO_MODER_MODE6_Pos)
				  |(1UL<<GPIO_MODER_MODE7_Pos)
				  |(1UL<<GPIO_MODER_MODE11_Pos)
				  |(1UL<<GPIO_MODER_MODE9_Pos)
				  |(1UL<<GPIO_MODER_MODE10_Pos)
			);

	/* 0: Output push-pull  */
	GPIOA->OTYPER&=~((1UL<<GPIO_OTYPER_OT4_Pos)
					|(1UL<<GPIO_OTYPER_OT5_Pos)
					|(1UL<<GPIO_OTYPER_OT6_Pos)
					|(1UL<<GPIO_OTYPER_OT7_Pos)
					|(1UL<<GPIO_OTYPER_OT8_Pos)
					|(1UL<<GPIO_OTYPER_OT9_Pos)
					|(1UL<<GPIO_OTYPER_OT10_Pos)
			);


    /* 00: Low speed, 2 bit fields */
	GPIOA->OSPEEDR&=~((0b11<<GPIO_OSPEEDR_OSPEED4_Pos)
					 |(0b11<<GPIO_OSPEEDR_OSPEED5_Pos)
					 |(0b11<<GPIO_OSPEEDR_OSPEED6_Pos)
					 |(0b11<<GPIO_OSPEEDR_OSPEED7_Pos)
					 |(0b11<<GPIO_OSPEEDR_OSPEED8_Pos)
					 |(0b11<<GPIO_OSPEEDR_OSPEED9_Pos)
					 |(0b11<<GPIO_OSPEEDR_OSPEED10_Pos)
			);
    /* 00: No pull-up, pull-down, 2 bit fields */
	GPIOA->PUPDR&=~((0b11<<GPIO_PUPDR_PUPD4_Pos)
			       |(0b11<<GPIO_PUPDR_PUPD5_Pos)
				   |(0b11<<GPIO_PUPDR_PUPD6_Pos)
				   |(0b11<<GPIO_PUPDR_PUPD7_Pos)
				   |(0b11<<GPIO_PUPDR_PUPD8_Pos)
				   |(0b11<<GPIO_PUPDR_PUPD9_Pos)
				   |(0b11<<GPIO_PUPDR_PUPD10_Pos)
			);

return STATUS_OK;
}