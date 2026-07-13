
#ifndef TIMER_8_H_
#define TIMER_8_H_

#include "peripheral_configs.h"

#define TIM8_CLOCK_VALUE_HZ   ((APB2_PRESCALER_VALUE == 1U) ? \
                            APB2_CLOCK_VALUE_HZ :             \
                            (APB2_CLOCK_VALUE_HZ * 2U))

/* tclk is sys_clk/1 = 100MHz  /
 * update_frequency = TIM8_CLK / ((PSC + 1) * (ARR + 1)
*/
#define TIM8_PSC_VALUE     10000
#define TIM8_PERIOD_TICKS  10000
void TIM8_CH1_CH2_PC6_PC7_OutputCompare_Init(void);

#endif /* TIMER_8_H_ */