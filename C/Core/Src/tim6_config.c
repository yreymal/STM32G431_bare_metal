#include "tim6_config.h"

volatile uint32_t g_ms_ticks = 0;
/* tclk is equal sys clk = 64MHz /
 * update_frequency = TIM6_CLK / ((PSC + 1) * (ARR + 1)

   1s = (64*10^6)/((x+1)*(y+1));
   (64*10^6) = (x+1)*(y+1)
   x = 64 * 10^2, y(ARR) = 10^4
*/
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
   TIM6->PSC = (6400 - 1);
   
   /* set auto-reload register ARR */
   TIM6->ARR = (10000 - 1);

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

     /* increase g_ms_ticks by 1 */
     ++g_ms_ticks;
     if(g_ms_ticks>9)
     g_ms_ticks = 0;
  }
}