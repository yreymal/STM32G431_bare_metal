#include "sys_tick.h"

static volatile uint32_t g_ms_counter = 0U;

void SysTick_1_Init_ms(uint32_t hclk_hz){

    if (hclk_hz < SYS_TICK_FREQ)
        return;
    
    /* reset ms counter*/
    g_ms_counter = 0;

    /* a value from what to count down to 0 */
    uint32_t reload_val = ((hclk_hz/SYS_TICK_FREQ ) -1 );

    /* turn off sys timer before configurating */
    SysTick->CTRL &= ~ SysTick_CTRL_ENABLE_Msk;
    
    /* 1: Processor clock (AHB) */
    SysTick->CTRL |=  SysTick_CTRL_CLKSOURCE_Msk;
   
    /* TICKINT = 1, will generate an interrupt */
    SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;
    
    /* set the reload value, from what to count down to 0 */
    if(reload_val> SysTick_LOAD_RELOAD_Msk)
        reload_val = SysTick_LOAD_RELOAD_Msk;
    SysTick->LOAD = reload_val;
    
     /* reset count val*/
    SysTick->VAL =0U;

    /* start the sys clk */
    SysTick->CTRL|= SysTick_CTRL_ENABLE_Msk;
    
}

uint32_t get_ms(void){
    return g_ms_counter;
}

void SysTick_Handler(void){
    ++g_ms_counter;
}