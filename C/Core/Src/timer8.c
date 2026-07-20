#include "timer8.h"
#include "stm32g431xx.h"

#define OUT_COM_CH1          6U      /* PC6 = TIM8_CH1 */
#define OUT_COM_CH2          7U      /* PC7 = TIM8_CH2 */

#define TIM8_GPIO_AF         4U      /* AF4 = TIM8_CH1 / TIM8_CH2 */

/*
 * These depend on your timer8.h values:
 *
 * TIM8_PSC_VALUE      = prescaler divider value, for example 10000
 * TIM8_PERIOD_TICKS   = timer period in CNT ticks, for example 5000
 */
#define TIM8_AUTORELOAD_VALUE   (TIM8_PERIOD_TICKS - 1U)
#define TIM8_COMPARE1_VALUE     2U//(TIM8_AUTORELOAD_VALUE / 2U)
#define TIM8_COMPARE2_VALUE     5U//(TIM8_AUTORELOAD_VALUE)

void TIM8_CH1_CH2_PC6_PC7_OutputCompare_Init(void)
{
    /*
     * 1. Enable GPIOC and TIM8 clocks.
     */
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOCEN;
    RCC->APB2ENR |= RCC_APB2ENR_TIM8EN;

    (void)RCC->AHB2ENR;
    (void)RCC->APB2ENR;

    /*
     * 2. Configure PC6 and PC7 as Alternate Function.
     *
     * PC6 = TIM8_CH1
     * PC7 = TIM8_CH2
     *
     * MODER = 10: alternate function mode
     */
    GPIOC->MODER &= ~((3U << (OUT_COM_CH1 * 2U)) |
                      (3U << (OUT_COM_CH2 * 2U)));

    GPIOC->MODER |=  ((2U << (OUT_COM_CH1 * 2U)) |
                      (2U << (OUT_COM_CH2 * 2U)));

    /*
     * Push-pull.
     */
    GPIOC->OTYPER &= ~((1U << OUT_COM_CH1) |
                       (1U << OUT_COM_CH2));

    /*
     * hight speed
     */
    GPIOC->OSPEEDR &= ~((3U << (OUT_COM_CH1 * 2U)) |
                        (3U << (OUT_COM_CH2 * 2U)));

    GPIOC->OSPEEDR |=  ((2U << (OUT_COM_CH1 * 2U)) |
                        (2U << (OUT_COM_CH2 * 2U)));

    /*
     * No pull-up, no pull-down.
     */
    GPIOC->PUPDR &= ~((3U << (OUT_COM_CH1 * 2U)) |
                      (3U << (OUT_COM_CH2 * 2U)));

    /*
     * PC6 and PC7 are in AFR[0].
     *
     * AF4 = TIM8_CH1 / TIM8_CH2
     */
    GPIOC->AFR[0] &= ~((0xFU << (OUT_COM_CH1 * 4U)) |
                       (0xFU << (OUT_COM_CH2 * 4U)));

    GPIOC->AFR[0] |=  ((TIM8_GPIO_AF << (OUT_COM_CH1 * 4U)) |
                       (TIM8_GPIO_AF << (OUT_COM_CH2 * 4U)));

    /*
     * 3. Stop TIM8 before configuration.
     */
    TIM8->CR1 &= ~TIM_CR1_CEN;

    /*
     * 4. Reset important TIM8 registers.
     */
    TIM8->CR1   = 0U;
    TIM8->CR2   = 0U;
    TIM8->SMCR  = 0U;
    TIM8->DIER  = 0U;
    TIM8->CCMR1 = 0U;
    TIM8->CCER  = 0U;
    TIM8->BDTR  = 0U;

    /*
     * 5. Timer base.
     *
     * PSC register value = divider - 1.
     *
     * Example:
     * TIM8_PSC_VALUE = 10000
     * TIM8->PSC = 9999
     */
    TIM8->PSC = TIM8_PSC_VALUE - 1U;

    /*
     * ARR register value = period ticks - 1.
     *
     * Example:
     * TIM8_PERIOD_TICKS = 5000
     * TIM8->ARR = 4999
     */
    TIM8->ARR = TIM8_AUTORELOAD_VALUE;

    /*
     * Start counting from 0.
     */
    TIM8->CNT = 0U;

    /*
     * CH1 compare point:
     * PC6 toggles in the middle of the timer period.
     */
    TIM8->CCR1 = TIM8_COMPARE1_VALUE;

    /*
     * CH2 compare point:
     * PC7 toggles when CNT reaches ARR.
     */
    TIM8->CCR2 = TIM8_COMPARE2_VALUE;

    /*
     * 6. Configure CH1 and CH2 as Output Compare.
     *
     * CC1S = 00: CH1 is output
     * CC2S = 00: CH2 is output
     */
    TIM8->CCMR1 &= ~(TIM_CCMR1_CC1S_Msk |
                     TIM_CCMR1_CC2S_Msk);

    /*
     * CH1 toggle mode.
     *
     * OC1M = 011: toggle when CNT == CCR1
     */
    TIM8->CCMR1 &= ~TIM_CCMR1_OC1M_Msk;
    TIM8->CCMR1 |=  (TIM_CCMR1_OC1M_0 |
                     TIM_CCMR1_OC1M_1);

    /*
     * CH2 toggle mode.
     *
     * OC2M = 011: toggle when CNT == CCR2
     */
    TIM8->CCMR1 &= ~TIM_CCMR1_OC2M_Msk;
    TIM8->CCMR1 |=  (TIM_CCMR1_OC2M_0 |
                     TIM_CCMR1_OC2M_1);

    /*
     * 7. Normal polarity.
     *
     * CC1P = 0: CH1 active high
     * CC2P = 0: CH2 active high
     */
    TIM8->CCER &= ~(TIM_CCER_CC1P |
                    TIM_CCER_CC2P);

    /*
     * Enable CH1 and CH2 outputs.
     */
    TIM8->CCER |= (TIM_CCER_CC1E |
                   TIM_CCER_CC2E);

    /*
     * 8. TIM8 is advanced timer.
     * Main Output Enable is required.
     */
    TIM8->BDTR |= TIM_BDTR_MOE;

    /*
     * 9. Load prescaler and ARR values.
     */
    TIM8->EGR = TIM_EGR_UG;

    /*
     * 10. Clear pending flags.
     */
    TIM8->SR = 0U;

    /*
     * 11. Start TIM8.
     */
    TIM8->CR1 |= TIM_CR1_CEN;
}