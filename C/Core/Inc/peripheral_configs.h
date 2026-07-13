/*
 * peripheral_configs.h
 *
 *  Created on: Jan 31, 2026
 *      Author: romanyarmak
 */

#ifndef INC_PERIPHERAL_CONFIGS_H_
#define INC_PERIPHERAL_CONFIGS_H_

#include <stdint.h>
#include <stm32g431xx.h>
#include "status.h"
#include "reg_bits_check.h"
#include "prescalers.h"

#define HSE_HZ        24000000UL
#define PLLM_DIV      3U
#define PLLR_DIV      2U  /* PLL could be only 2,4,6,8 “R” output clock frequency = VCO frequency / PLLR */
#define PLLM_DIV_REG_VAL (PLLM_DIV - 1)
#define PLLR_DIV_REG_VAL ((PLLR_DIV/2U) -1U)

#define PLLN_MUL      25U /* value can be only in rage PLLN 8 =< PLLN =< 127 */



#define VCO_CLK_HZ  ((HSE_HZ / PLLM_DIV) * PLLN_MUL)  /* 200000000UL */
#define SYSCLK_HZ   (VCO_CLK_HZ / PLLR_DIV)              /* 100000000UL */

#define AHB_CLOCK_VALUE_HZ            (SYSCLK_HZ / AHB_PRESCALER_VALUE)
#define APB1_CLOCK_VALUE_HZ           (AHB_CLOCK_VALUE_HZ / APB1_PRESCALER_VALUE)
#define APB2_CLOCK_VALUE_HZ           (AHB_CLOCK_VALUE_HZ / APB2_PRESCALER_VALUE)


/**
  * @brief  The application entry point.
  * @retval int
  */

int configureUserButton();

status_t ConfigureClock(void);

status_t initGPIOA5();

int delay(const uint32_t timer);

status_t configure_MCO_pinA8();







#endif /* INC_PERIPHERAL_CONFIGS_H_ */
