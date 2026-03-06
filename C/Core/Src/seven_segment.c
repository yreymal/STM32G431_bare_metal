/*
 * seven_segment.c
 *
 *  Created on: Jan 31, 2026
 *      Author: romanyarmak
 */

#include "seven_segment.h"

/* 	GPIO_ODR_OD4_Pos)	pin 4 - A				A
	GPIO_ODR_OD5_Pos	pin 5  -B			  F   B
	GPIO_ODR_OD6_Pos	pin 6  -C				G
	GPIO_ODR_OD7_Pos	pin 7  -D			  E	  C
	GPIO_ODR_OD8_Pos	pin 8  -E				D
	GPIO_ODR_OD9_Pos	pin 9  -F
	GPIO_ODR_OD10_Pos   pin 10 -G
 * */
#define ALL_8_DIGITS    (0b1111111<<4UL)
#define DIGIT_ONE 		(GPIO_ODR_OD5_Msk|GPIO_ODR_OD6_Msk)
#define DIGIT_TWO 		(~(GPIO_ODR_OD9_Msk|GPIO_ODR_OD6_Msk))
#define DIGIT_THREE		(~(GPIO_ODR_OD8_Msk|GPIO_ODR_OD9_Msk))
#define DIGIT_FOUR		(~(GPIO_ODR_OD4_Msk|GPIO_ODR_OD8_Msk|GPIO_ODR_OD7_Msk))
#define DIGIT_FIVE		(~(GPIO_ODR_OD5_Msk|GPIO_ODR_OD8_Msk))
#define DIGIT_SIX		(~GPIO_ODR_OD5_Msk)
#define DIGIT_SEVEN		(~(GPIO_ODR_OD9_Msk|GPIO_ODR_OD8_Msk|GPIO_ODR_OD7_Msk))
#define DIGIT_NINE		(~GPIO_ODR_OD8_Msk)

uint32_t segment_numbers[10] = {
		                ~GPIO_ODR_OD10_Msk,      //0
						(GPIO_ODR_OD5_Msk|GPIO_ODR_OD6_Msk),//1
						 ~(GPIO_ODR_OD9_Msk|GPIO_ODR_OD6_Msk),//2
						(~(GPIO_ODR_OD8_Msk|GPIO_ODR_OD9_Msk)),//3
						(~(GPIO_ODR_OD4_Msk|GPIO_ODR_OD8_Msk|GPIO_ODR_OD7_Msk)),//4
						(~(GPIO_ODR_OD5_Msk|GPIO_ODR_OD8_Msk)),//5
						(~GPIO_ODR_OD5_Msk),		//6
						((GPIO_ODR_OD4_Msk|GPIO_ODR_OD5_Msk|GPIO_ODR_OD6_Msk)),//7
						(0b1111111<<4UL),		//8
						~GPIO_ODR_OD8_Msk		//9
						};


uint32_t seg_numbers[] = {(1UL<<GPIO_ODR_OD4_Pos),//pin 4 - A
						 1UL<<GPIO_ODR_OD5_Pos,//pin 5  -B
						 1UL<<GPIO_ODR_OD6_Pos,//pin 6  -C
						 1UL<<GPIO_ODR_OD7_Pos,//pin 7  -D
						 1UL<<GPIO_ODR_OD8_Pos,//pin 8  -E
						 1UL<<GPIO_ODR_OD9_Pos,//pin 9  -F
						 1UL<<GPIO_ODR_OD10_Pos//pin 10 -G
						};

/*PA4-PA10
 * OUTPUT - configuration must be push-pull */
status_t configure_7_seg_pins(void){

	/* check weather port A clock already enabled, if not, turn it on */
	if(!(RCC->AHB2ENR & (1UL<<RCC_AHB2ENR_GPIOAEN_Pos))){
		RCC->AHB2ENR|=(1UL<<RCC_AHB2ENR_GPIOAEN_Pos);
	}

	/* MODER has 2 bits fields, set 0 to all pins from PA4 till PA10 */
	GPIOA->MODER&=~((0b11<<GPIO_MODER_MODE4_Pos)
			       |(0b11<<GPIO_MODER_MODE5_Pos)
				   |(0b11<<GPIO_MODER_MODE6_Pos)
				   |(0b11<<GPIO_MODER_MODE7_Pos)
				   |(0b11<<GPIO_MODER_MODE8_Pos)
				   |(0b11<<GPIO_MODER_MODE9_Pos)
				   |(0b11<<GPIO_MODER_MODE10_Pos)
			);
	/* 01: General purpose output mode */
	GPIOA->MODER|=((1UL<<GPIO_MODER_MODE4_Pos)
				  |(1UL<<GPIO_MODER_MODE5_Pos)
				  |(1UL<<GPIO_MODER_MODE6_Pos)
				  |(1UL<<GPIO_MODER_MODE7_Pos)
				  |(1UL<<GPIO_MODER_MODE8_Pos)
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
