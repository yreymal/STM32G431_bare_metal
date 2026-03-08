/*
 * board_init.cpp
 *
 *  Created on: March 3, 2026
 *      Author: romanyarmak
 */


#include "status.hpp"
#include "stm32g431xx.h"
#include "bit_check.hpp"   // isBitSet(), isBitZero(), isValueSet()
#include "board_init.hpp"

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

    /* configure pin 13 as input without pulling up/down,
	 * because there is HW pull down resistor for the button on board
	 * |00| - Input mode */
    GPIOC->MODER &= ~GPIO_MODER_MODE13_Msk;

   /* it's important to turn off pull up/down, because of external HW pull down
	 * 00: No pull-up, pull-down */
    GPIOC->PUPDR &= ~GPIO_PUPDR_PUPD13_Msk;

    return Status::kOk;
}

[[nodiscard]] Status configureClock()
{   /* function return check */
    Status st = Status::kOk;

    //----------------------------------------------------------------------------
    // 1. Enable HSI and switch temporarily to HSI
    //----------------------------------------------------------------------------
    
	/* for safety enable and switch temporarily to HSI */
	RCC->CR|=(1UL<<RCC_CR_HSION_Pos);
    /* wait until the HSI rdy bit is set, or return with error status */
    st = isBitSet(&RCC->CR, RCC_CR_HSIRDY_Msk, kReadyTimeout);
    if (st != Status::kOk)
    {
        return st;
    }

    /* switch clock source from  HSI16, 
	* clear SW bits, then write bits for HSI in CFGR register
	*/
    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW_Msk) | RCC_CFGR_SW_HSI;
    /* wait until the HSI clock set, or return with error status*/
    st = isValueSet(&RCC->CFGR, RCC_CFGR_SWS_Msk, kSwitchTimeout, RCC_CFGR_SWS_HSI);
    if (st != Status::kOk)
    {
        return st;
    }

    //----------------------------------------------------------------------------
    // 2. Enable HSE
    //----------------------------------------------------------------------------
  
    /* turning ON HSE */
    RCC->CR|=(1UL<<RCC_CR_HSEON_Pos);
    /* wait until the HSE rdy bit is set, or return with error status */
    st = isBitSet(&RCC->CR, RCC_CR_HSERDY_Msk, kReadyTimeout);
    if (st != Status::kOk)
    {
        return st;
    }

    //----------------------------------------------------------------------------
    // 3. Disable PLL before reconfiguration
    //----------------------------------------------------------------------------
    /* turning off PLL before set its configuration */
    RCC->CR &= ~RCC_CR_PLLON_Msk;

    /* wait until the PLL rdy bit is set to 0, or return with error status */
    st = isBitZero(&RCC->CR, RCC_CR_PLLRDY_Msk, kReadyTimeout);
    if (st != Status::kOk)
    {
        return st;
    }

    //----------------------------------------------------------------------------
    // 4. Configure FLASH latency before increasing SYSCLK
    //----------------------------------------------------------------------------

    /* clean latency bits
     * setting 2 WS for FLASH
	 * 2 WC if CLCK<= 90 MHz, we use 64 MHz */
    FLASH->ACR = (FLASH->ACR & ~FLASH_ACR_LATENCY_Msk) | FLASH_ACR_LATENCY_2WS;

    //----------------------------------------------------------------------------
    // 5. Set bus prescalers
    //----------------------------------------------------------------------------

    /* clear peripheria prescale bits */
	/* PPR1 - APB1, PPR2 - APB2, HPRE - AHB */
    RCC->CFGR &= ~(RCC_CFGR_PPRE1_Msk | RCC_CFGR_PPRE2_Msk | RCC_CFGR_HPRE_Msk);

    //----------------------------------------------------------------------------
    // 6. Configure PLL: HSE=24 MHz -> SYSCLK=64 MHz
    //
    // PLLM = 3   => 24 / 3 = 8 MHz
    // PLLN = 16  => 8 * 16 = 128 MHz
    // PLLR = 2   => 128 / 2 = 64 MHz
    //----------------------------------------------------------------------------

    /* clear multi-bits fields before setting */
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

    /*turning ON PLL and wait for PLLRDY flag set as 1, or return with error status*/
    RCC->CR |= RCC_CR_PLLON_Msk;
    st = isBitSet(&RCC->CR, RCC_CR_PLLRDY_Msk, kReadyTimeout);
    if (st != Status::kOk)
    {
        return st;
    }

    //----------------------------------------------------------------------------
    // 8. Switch SYSCLK to PLL
    //----------------------------------------------------------------------------
    /* clear SW CFGR multi-bits fields before setting and write bits for PLL as SYSCLK */
    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW_Msk) | RCC_CFGR_SW_PLL;

    /* wait until the SWS set as PLL, or return with error status */
    st = isValueSet(&RCC->CFGR, RCC_CFGR_SWS_Msk, kReadyTimeout, RCC_CFGR_SWS_PLL);
    if (st != Status::kOk)
    {
        return st;
    }

    //----------------------------------------------------------------------------
    // 9. Disable HSI, no longer needed
    //----------------------------------------------------------------------------
    /*turning off HSI */
    RCC->CR &= ~RCC_CR_HSION_Msk;

    /* wait until HSI status set as turned off, or return with error status */
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

    /* configure PA5 as general purpose output mode: 01 */
    GPIOA->MODER &= ~GPIO_MODER_MODE5_Msk;
    GPIOA->MODER |=  (1UL << GPIO_MODER_MODE5_Pos);

    Status st = isValueSet(&GPIOA->MODER, GPIO_MODER_MODE5_Msk, kReadyTimeout, (1UL << GPIO_MODER_MODE5_Pos));
    if (st != Status::kOK)
    {
        return st;
    }

    /* output type: push-pull = 0 */
    GPIOA->OTYPER &= ~GPIO_OTYPER_OT5_Msk;

    st = isBitZero(&GPIOA->OTYPER, GPIO_OTYPER_OT5_Msk, kReadyTimeout);
    if (st != Status::kOK)
    {
        return st;
    }

    /* no pull-up, no pull-down */
    GPIOA->PUPDR &= ~GPIO_PUPDR_PUPD5_Msk;

    st = isValueSet(&GPIOA->PUPDR, GPIO_PUPDR_PUPD5_Msk, kReadyTimeout, 0x0U);
    if (st != Status::kOK)
    {
        return st;
    }

    /* low speed */
    GPIOA->OSPEEDR &= ~GPIO_OSPEEDR_OSPEED5_Msk;

    st = isValueSet(&GPIOA->OSPEEDR, GPIO_OSPEEDR_OSPEED5_Msk, kReadyTimeout, 0x0U);
    if (st != Status::kOK)
    {
        return st;
    }

    return Status::kOK;
}

//TODO: delay function

}  // namespace board