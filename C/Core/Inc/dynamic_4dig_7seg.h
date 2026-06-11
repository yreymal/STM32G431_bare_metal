/*
 * dynamic_4dig_7seg.h
 *
 *  Created on: Jun 11, 2026
 *      Author: romanyarmak
 */

#ifndef DYN_4DIG_7SEG_H_
#define DYN_4DIG_7SEG_H_

#include <stm32g431xx.h>
#include "status.h"
#include "seven_segment.h"

 /*    

    DIG1     DIG2    DIG3   DIG4
    PC12     PC13    PC14   PC15
              _       _      
      |       _|      _|    |_|
      |      |_       _|      |


 */

 /* for dynamic 4 digit display we have to toggle + voltage for each its corresponding pin in sequence /
    PC12 ON, all athother - OFF
    PC13 ON, the rest - OFF
    PC14 ON, the rest - OFF
    PC15 ON, the rest - OFF
 */

 /*  15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0                                          \
 	  D1 D2 D3 D4 _  _  _ _ _ _ _ _ _ _ _ _   
  t1  1  0  0  0 _ _ _ _ _ _ _ _ _ _ _ _ _	  those 4 bits have to be shifted(<<12) \
  t2	0  1  0  0 _ _ _ _ _ _ _ _ _ _ _ _ _     because pins PC0-PC11 are not used for 7 segment 4 digit common ANOD display \
  t3  0  0  1  0 _ _ _ _ _ _ _ _ _ _ _ _ _
  t4  0  0  0  1 _ _ _ _ _ _ _ _ _ _ _ _ _
  t - time period
 */


 #define DIGIT1_Msk (1UL<<2UL)
 #define DIGIT2_Msk (1UL<<3UL)
 #define DIGIT3_Msk (1UL<<10UL)
 #define DIGIT4_Msk (1UL<<12UL)

#define ALL_DIGITS (DIGIT1_Msk|DIGIT2_Msk|DIGIT3_Msk|DIGIT4_Msk)
extern const uint32_t digit_0_4[4];

void show_digit_on_display(uint8_t digit, uint8_t data);
status_t init_portc();

#endif /* DYN_4DIG_7SEG_H_ */