/*
 * board_init.cpp
 *
 *  Created on: March 3, 2026
 *      Author: romanyarmak
 */

#include "board_init.hpp"
#include "stm32g4xx.h"
#include "bit_check.hpp"   // isBitSet(), isBitZero(), isValueSet()

namespace board
{
namespace
{
constexpr uint32_t kReadyTimeout = 0x5000U;
constexpr uint32_t kSwitchTimeout = 0x1000U;
}  // namespace

[[nodiscard]] Status configureUserButton()
{
    /* user button is connected to port C, pin 13 */

    /* enable clock for GPIOC */
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOCEN_Msk;

    /* configure pin 13 as input mode */
    GPIOC->MODER &= ~GPIO_MODER_MODE13_Msk;

    /* no pull-up / no pull-down
     * external HW pull-down resistor is already present on board
     */
    GPIOC->PUPDR &= ~GPIO_PUPDR_PUPD13_Msk;

    return Status::kOk;
}

[[nodiscard]] Status configureClock()
{
    Status st = Status::kOk;

    //----------------------------------------------------------------------------
    // 1. Enable HSI and switch temporarily to HSI
    //----------------------------------------------------------------------------
    RCC->CR |= RCC_CR_HSION_Msk;

    st = isBitSet(&RCC->CR, RCC_CR_HSIRDY_Msk, kReadyTimeout);
    if (st != Status::kOk)
    {
        return st;
    }

    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW_Msk) | RCC_CFGR_SW_HSI;

    st = isValueSet(&RCC->CFGR, RCC_CFGR_SWS_Msk, kSwitchTimeout, RCC_CFGR_SWS_HSI);
    if (st != Status::kOk)
    {
        return st;
    }

    //----------------------------------------------------------------------------
    // 2. Enable HSE
    //----------------------------------------------------------------------------
    RCC->CR |= RCC_CR_HSEON_Msk;

    st = isBitSet(&RCC->CR, RCC_CR_HSERDY_Msk, kReadyTimeout);
    if (st != Status::kOk)
    {
        return st;
    }

    //----------------------------------------------------------------------------
    // 3. Disable PLL before reconfiguration
    //----------------------------------------------------------------------------
    RCC->CR &= ~RCC_CR_PLLON_Msk;

    st = isBitZero(&RCC->CR, RCC_CR_PLLRDY_Msk, kReadyTimeout);
    if (st != Status::kOk)
    {
        return st;
    }

    //----------------------------------------------------------------------------
    // 4. Configure FLASH latency before increasing SYSCLK
    //----------------------------------------------------------------------------
    FLASH->ACR = (FLASH->ACR & ~FLASH_ACR_LATENCY_Msk) | FLASH_ACR_LATENCY_2WS;

    //----------------------------------------------------------------------------
    // 5. Set bus prescalers to /1
    //----------------------------------------------------------------------------
    RCC->CFGR &= ~(RCC_CFGR_PPRE1_Msk | RCC_CFGR_PPRE2_Msk | RCC_CFGR_HPRE_Msk);

    //----------------------------------------------------------------------------
    // 6. Configure PLL: HSE=24 MHz -> SYSCLK=64 MHz
    //
    // PLLM = 3   => 24 / 3 = 8 MHz
    // PLLN = 16  => 8 * 16 = 128 MHz
    // PLLR = 2   => 128 / 2 = 64 MHz
    //----------------------------------------------------------------------------
    RCC->PLLCFGR &= ~(RCC_PLLCFGR_PLLSRC_Msk |
                      RCC_PLLCFGR_PLLM_Msk   |
                      RCC_PLLCFGR_PLLN_Msk   |
                      RCC_PLLCFGR_PLLR_Msk);

    RCC->PLLCFGR |= ((0b11U << RCC_PLLCFGR_PLLSRC_Pos) |  // HSE as PLL source
                     (0b10U << RCC_PLLCFGR_PLLM_Pos)   |  // PLLM = 3
                     (16U   << RCC_PLLCFGR_PLLN_Pos)   |  // PLLN = 16
                     (0U    << RCC_PLLCFGR_PLLR_Pos)   |  // PLLR = 2
                     RCC_PLLCFGR_PLLREN);

    //----------------------------------------------------------------------------
    // 7. Enable PLL
    //----------------------------------------------------------------------------
    RCC->CR |= RCC_CR_PLLON_Msk;

    st = isBitSet(&RCC->CR, RCC_CR_PLLRDY_Msk, kReadyTimeout);
    if (st != Status::kOk)
    {
        return st;
    }

    //----------------------------------------------------------------------------
    // 8. Switch SYSCLK to PLL
    //----------------------------------------------------------------------------
    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW_Msk) | RCC_CFGR_SW_PLL;

    st = isValueSet(&RCC->CFGR, RCC_CFGR_SWS_Msk, kReadyTimeout, RCC_CFGR_SWS_PLL);
    if (st != Status::kOk)
    {
        return st;
    }

    //----------------------------------------------------------------------------
    // 9. Disable HSI if no longer needed
    //----------------------------------------------------------------------------
    RCC->CR &= ~RCC_CR_HSION_Msk;

    st = isBitZero(&RCC->CR, RCC_CR_HSIRDY_Msk, kReadyTimeout);
    if (st != Status::kOk)
    {
        return st;
    }

    return st;
}

[[nodiscard]] Status initGpioA5()
{
    /* enable clock for GPIOA */
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN_Msk;

    /* set PA5 to general purpose output mode */
    GPIOA->MODER &= ~GPIO_MODER_MODE5_Msk;
    GPIOA->MODER |= (0x1UL << GPIO_MODER_MODE5_Pos);

    /* push-pull */
    GPIOA->OTYPER &= ~GPIO_OTYPER_OT5_Msk;

    /* no pull-up / no pull-down */
    GPIOA->PUPDR &= ~GPIO_PUPDR_PUPD5_Msk;

    /* low speed */
    GPIOA->OSPEEDR &= ~GPIO_OSPEEDR_OSPEED5_Msk;

    return Status::kOk;
}

//TODO: delay function

}  // namespace board