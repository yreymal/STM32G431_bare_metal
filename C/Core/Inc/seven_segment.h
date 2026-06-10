/*
 * seven_segment.h
 *
 *  Created on: Jan 24, 2026
 *      Author: romanyarmak
 */
#include <stdint.h>
#include <stm32g431xx.h>
#include "status.h"

#ifndef INC_SEVEN_SEGMENT_H_
#define INC_SEVEN_SEGMENT_H_
/*
*
*
/* for 7 segment 4 digit common ANOD display /
*
*
*/

/* PORT A in STM32G4 NUCLEO board */
/* 	GPIO_ODR_OD4_Pos	pin 4 - A				A
	GPIO_ODR_OD5_Pos	pin 5  -B			  F   B
	GPIO_ODR_OD6_Pos	pin 6  -C				G
	GPIO_ODR_OD7_Pos	pin 7  -D			  E	  C
	GPIO_ODR_OD11_Pos	pin 11  -E				D
	GPIO_ODR_OD9_Pos	pin 9  -F
	GPIO_ODR_OD10_Pos   pin 10 -G
 * */
#define SEG_7_ALL_MSK   (0b11101111<<4UL)/*(0bxxx_xxxx<<4UL)  PIN PA8 is reserved /
									  ~ is used because of common ANOD, so we have to send 0 to tunr a segment	*/
 
 /*  11 10 9 8 7 6 5 4  3 2 1 0
 	  E  G F _ D C B A  _ _ _ _   PIN PA8 is reserved
      1  1 1 _ 1 1 1 1  _ _ _ _	  those 4 bits have to be shifted(<<4)
	                              because pins PA0-PA3 are not used for 7 segment 4 digit common ANOD display */
	  
	  
 
#define DIGIT_ZERO		((GPIO_ODR_OD10_Msk) & SEG_7_ALL_MSK)
#define DIGIT_ONE 		~(GPIO_ODR_OD5_Msk|GPIO_ODR_OD6_Msk)
#define DIGIT_TWO 		((GPIO_ODR_OD9_Msk|GPIO_ODR_OD6_Msk) & SEG_7_ALL_MSK)

#define DIGIT_THREE		((GPIO_ODR_OD11_Msk|GPIO_ODR_OD9_Msk) & SEG_7_ALL_MSK)

#define DIGIT_FOUR		((GPIO_ODR_OD4_Msk|GPIO_ODR_OD11_Msk|GPIO_ODR_OD7_Msk) & SEG_7_ALL_MSK)
#define DIGIT_FIVE		((GPIO_ODR_OD5_Msk|GPIO_ODR_OD11_Msk) & SEG_7_ALL_MSK)
#define DIGIT_SIX		((GPIO_ODR_OD5_Msk) & SEG_7_ALL_MSK)
#define DIGIT_SEVEN		~((GPIO_ODR_OD4_Msk|GPIO_ODR_OD5_Msk|GPIO_ODR_OD6_Msk))
#define DIGIT_EIGHT		~(SEG_7_ALL_MSK)
#define DIGIT_NINE		((GPIO_ODR_OD11_Msk) & SEG_7_ALL_MSK)



static const uint32_t segment_numbers[10] = {
		                DIGIT_ZERO,     
						DIGIT_ONE,
						DIGIT_TWO,
						DIGIT_THREE,
						DIGIT_FOUR,
						DIGIT_FIVE,
						DIGIT_SIX,		
						DIGIT_SEVEN,
						DIGIT_EIGHT,	
						DIGIT_NINE
						};




#endif /* INC_SEVEN_SEGMENT_H_ */
