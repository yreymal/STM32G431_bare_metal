#include "dynamic_4dig_7seg.h"



static const uint32_t digit_masks[4] =
{
    DIGIT1_Msk,
    DIGIT2_Msk,
    DIGIT3_Msk,
    DIGIT4_Msk
};


/*  15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0                                          \
    D1 D2 D3 D4 _  _  _ _ _ _ _ _ _ _ _ _
 t1  1  0  0  0 _ _ _ _ _ _ _ _ _ _ _ _ _	  those 4 bits have to be shifted(<<12) \
 t2	 0  1  0  0 _ _ _ _ _ _ _ _ _ _ _ _ _     because pins PC0-PC11 are not used for 7 segment 4 digit common ANOD display \
 t3  0  0  1  0 _ _ _ _ _ _ _ _ _ _ _ _ _
 t4  0  0  0  1 _ _ _ _ _ _ _ _ _ _ _ _ _
 t - time period
 */
void show_digit_on_dis_pos(uint16_t pos, uint8_t data)
{
   
    GPIOA->ODR &= ~ SEG_7_ALL_MSK;
    GPIOA->ODR =  segment_numbers[data];

    /* switching which digit to turn on 1-4 */
    GPIOC->ODR &= ~ ALL_DIGITS;
    GPIOC->ODR |= digit_masks[pos];


}

void segments_init(void* par){
  display_refresh_par *display_par = (display_refresh_par*)par;
  


    display_par->digits_1_4[0] = 0;
    display_par->digits_1_4[1] = 0;
    display_par->digits_1_4[2]= 0;
    display_par->digits_1_4[3] =0;
 
   display_par->counter = 0;

}
void calculate_segments_for_digit(void* par)
{   
  
  //if(((display_refresh_par*)par)->digit_number)
  display_refresh_par *display_par = (display_refresh_par*)par;
  
  if (display_par->digit_number > 9999U) {
        display_par->digit_number = 9999U;
    }
    
   
    display_par->digits_1_4[0] = display_par->digit_number / 1000U;
    display_par->digits_1_4[1] = (display_par->digit_number / 100U) % 10U;
    display_par->digits_1_4[2]= (display_par->digit_number / 10U)  % 10U;
    display_par->digits_1_4[3] = display_par->digit_number % 10U;

    for(uint8_t i = 0; i <3;++i){
    if(display_par->digits_1_4[i] == 0){
        display_par->digits_1_4[i] = DISPLAY_BLANK;
    }
    else
    {
        return;
    }
}

}

void show_digit_on_display(void* par)
{
   display_refresh_par *display_par = (display_refresh_par *)par;
   /* digit to show */
    uint8_t i = display_par->counter;
    if(display_par->digits_1_4[i]!=DISPLAY_BLANK){
    GPIOA->BSRR  = (SEG_7_ALL_MSK << 16);
    GPIOA->BSRR =  segment_numbers[display_par->digits_1_4[i]];

    /* switching which digit to turn on 1-4 */
  
    GPIOC->BSRR = (ALL_DIGITS << 16);
    //GPIOC->ODR |= digit_masks[(par->digit_number)/3];
     GPIOC->BSRR |= digit_masks[i];
    }
++display_par->counter;
if(display_par->counter>3)
    display_par->counter = 0;
   }


void test_display(void* par){
    
   GPIOA->BSRR  = (SEG_7_ALL_MSK << 16);
    GPIOA->BSRR =  segment_numbers[4];
  
    /* switching which digit to turn on 1-4 */
    GPIOC->BSRR = (ALL_DIGITS << 16);
    GPIOC->BSRR = digit_masks[3];
}


// void stopwatch_start(uint8_t digit)
// {

//     GPIOA->ODR &= ~ SEG_7_ALL_MSK;
//     GPIOA->ODR =  segment_numbers[data];

//     /* switching which digit to turn on 1-4 */
//     GPIOC->ODR &= ~ ALL_DIGITS;
//     GPIOC->ODR |= digit_masks[digit];


// }

status_t init_portc()
{
    /* Enable port C peripheral clocking, if it wasn't enable yet*/
    if (!(RCC->AHB2ENR & RCC_AHB2ENR_GPIOCEN_Msk))
        RCC->AHB2ENR |= RCC_AHB2ENR_GPIOCEN_Msk;

    /* configurate pins 12-15 for general output mode */
    GPIOC->MODER &= ~(GPIO_MODER_MODE12_Msk |
                  GPIO_MODER_MODE10_Msk |
                  GPIO_MODER_MODE2_Msk |
                  GPIO_MODER_MODE3_Msk);

GPIOC->MODER |=  (GPIO_MODER_MODE12_0 |
                  GPIO_MODER_MODE10_0 |
                  GPIO_MODER_MODE2_0 |
                  GPIO_MODER_MODE3_0);
    
    /* set push/pull output type
        0: Output push-pull 
     */
    GPIOC->OTYPER &= ~(GPIO_OTYPER_OT12_Msk|GPIO_OTYPER_OT10_Msk|GPIO_OTYPER_OT2_Msk|GPIO_OTYPER_OT3_Msk);

    /* set output speed
       00: Low speed
    */
    GPIOC->OSPEEDR &= ~(GPIO_OSPEEDR_OSPEED12_Msk|GPIO_OSPEEDR_OSPEED10_Msk|GPIO_OSPEEDR_OSPEED12_Msk|GPIO_OSPEEDR_OSPEED13_Msk);

GPIOC->PUPDR &= ~(GPIO_PUPDR_PUPD12_Msk |
                  GPIO_PUPDR_PUPD10_Msk |
                  GPIO_PUPDR_PUPD2_Msk |
                  GPIO_PUPDR_PUPD3_Msk);

    return STATUS_OK;
}
