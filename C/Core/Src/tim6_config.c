#include "tim6_config.h"
#include "dynamic_4dig_7seg.h"

volatile uint8_t g_seconds_ticks = 0;
volatile uint8_t g_tim6_is_running = 0;

status_t tim6_init(){
    /* enable peripheral clock for TIM6 on APB1 bus */
    RCC->APB1ENR1|= RCC_APB1ENR1_TIM6EN_Msk;

   /* dummy reading after enabling the peripheral clock /
      updating the structure in memory
    */
   (void)RCC->APB1ENR1;

   /* disable TIM6 before configuration */
   TIM6->CR1&= ~TIM_CR1_CEN_Msk;
    
   /* set timer prescaler PSC */
   TIM6->PSC = (PSC_DEF - 1);
   
   /* set auto-reload register ARR */
   TIM6->ARR = (ARR_DEF - 1);

   /* generate update event manually /
    force the prescaler value to be loaded immediately
    */
   TIM6->EGR|= TIM_EGR_UG_Msk;

   /* clear update interrupt flag */
   TIM6->SR &=~ TIM_SR_UIF_Msk;

   /* enable update interrupt for TIM6 */
   TIM6->DIER|= TIM_DIER_UIE_Msk;

   /* set interrupt priority */
   NVIC_SetPriority(TIM6_DAC_IRQn, 2);

   /* enable TIM6 interrupt in NVIC */
   NVIC_EnableIRQ(TIM6_DAC_IRQn);

    /* start TIM6 counter */
    TIM6->CR1|= TIM_CR1_CEN_Msk;

   return STATUS_OK;
}

/* TIM6 interrupt handler, name of function to overrite can be found in startup_stm32g431xx.s  */
void TIM6_DAC_IRQHandler(void){
 
  /* Because of the interrupt line is shared between TIM6 and DAC, /
   we must check if update interrupt flag is set by TIM6,
   and if update interrupt is enabled (UIE)
   */
 if((TIM6->SR & TIM_SR_UIF_Msk) && (TIM6->DIER & TIM_DIER_UIE_Msk)){
     /* clear update interrupt flag */
     TIM6->SR &=~ TIM_SR_UIF_Msk;

     /* increase g_seconds_ticks by 1 */
     ++g_seconds_ticks;
     ++g_tim6_is_running;
     if(g_seconds_ticks>9)
     g_seconds_ticks = 0;
  }
}

  void toggle_tim6(void){
      /* start TIM6 counter */
      TIM6->CR1^= TIM_CR1_CEN_Msk;
  }

  uint8_t get_seconds_tim6(){
    return g_seconds_ticks;
  }

   uint8_t get_digit(){
    return g_tim6_is_running;
  }

  void store_seconds_tim6(void *par){
    display_refresh_par *dp = (display_refresh_par *)par;
     (*dp).digit_number = g_seconds_ticks;
  }
