/*
 * Board-level clock and MCO setup.
 *
 * This file performs the sequence required to move safely from the reset HSI
 * clock to the external-crystal PLL clock. Each hardware wait is bounded and
 * every failure is returned to main instead of being ignored.
 */
#include "board.hpp"

#include "clock_config.hpp"
#include "register_utils.hpp"
#include "stm32g431xx.h"

namespace board {

Status configureClockOutput()
{
    // Section A: give GPIOA a bus clock. `|=` sets only the GPIOAEN bit and
    // preserves every other peripheral-enable bit already present in AHB2ENR.
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN_Msk;

    // Section B: configure PA8 as alternate function mode (binary 10).
    // First AND with the inverted mask clears only PA8's two mode bits; then OR
    // inserts the desired value. This safe pattern is used for normal R/W fields.
    GPIOA->MODER = (GPIOA->MODER & ~GPIO_MODER_MODE8_Msk)
                 | (2UL << GPIO_MODER_MODE8_Pos);

    // Push-pull output, very-high speed, no internal pull resistor, AF0.
    GPIOA->OTYPER &= ~GPIO_OTYPER_OT8_Msk;
    GPIOA->OSPEEDR = (GPIOA->OSPEEDR & ~GPIO_OSPEEDR_OSPEED8_Msk)
                   | (3UL << GPIO_OSPEEDR_OSPEED8_Pos);
    GPIOA->PUPDR &= ~GPIO_PUPDR_PUPD8_Msk;
    GPIOA->AFR[1] &= ~GPIO_AFRH_AFSEL8_Msk;

    // Section C: select SYSCLK as MCO source and divide it by 16. With the
    // configured 100 MHz SYSCLK, the oscilloscope should measure 6.25 MHz.
    RCC->CFGR = (RCC->CFGR & ~(RCC_CFGR_MCOPRE_Msk | RCC_CFGR_MCOSEL_Msk))
              | RCC_CFGR_MCOPRE_DIV16
              | (1UL << RCC_CFGR_MCOSEL_Pos);

    // Section D: read back the field. This checks the register accepted the
    // configuration; it cannot prove that the physical clock frequency is right.
    constexpr std::uint32_t expectedMco = RCC_CFGR_MCOPRE_DIV16
                                        | (1UL << RCC_CFGR_MCOSEL_Pos);
    return register_utils::waitForValue(
        &RCC->CFGR,
        RCC_CFGR_MCOPRE_Msk | RCC_CFGR_MCOSEL_Msk,
        expectedMco);
}

Status configureClock()
{
    // Makes waitForSet/waitForValue/waitForClear usable without repeatedly
    // writing the register_utils:: prefix inside this function only.
    using namespace register_utils;

    // Step 1: enable the known 16 MHz internal oscillator and wait until stable.
    // It is our temporary clock while HSE/PLL registers are being changed.
    RCC->CR |= RCC_CR_HSION_Msk;
    // `auto` asks the compiler to infer Status from waitForSet's return type.
    auto status = waitForSet(&RCC->CR, RCC_CR_HSIRDY_Msk);
    if (status != Status::kOk) {
        return status;
    }

    // Step 2: request HSI as SYSCLK, then inspect SWS (status), not merely SW
    // (our request). Hardware may need several cycles to complete the switch.
    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW_Msk) | RCC_CFGR_SW_HSI;
    status = waitForValue(&RCC->CFGR, RCC_CFGR_SWS_Msk, RCC_CFGR_SWS_HSI);
    if (status != Status::kOk) {
        return status;
    }

    // Step 3: configure and start the 24 MHz external source. `if constexpr`
    // is resolved at compile time, so only the selected branch remains in code.
    if constexpr (clock_config::kHseBypass) {
        RCC->CR |= RCC_CR_HSEBYP_Msk;
    } else {
        RCC->CR &= ~RCC_CR_HSEBYP_Msk;
    }
    RCC->CR |= RCC_CR_HSEON_Msk;
    status = waitForSet(&RCC->CR, RCC_CR_HSERDY_Msk);
    if (status != Status::kOk) {
        return status;
    }

    // Step 4: the PLL must be disabled and no longer ready before PLLCFGR may
    // be modified. The timeout also covers a PLL that refuses to stop.
    RCC->CR &= ~RCC_CR_PLLON_Msk;
    status = waitForClear(&RCC->CR, RCC_CR_PLLRDY_Msk);
    if (status != Status::kOk) {
        return status;
    }

    // Step 5: increase Flash wait states before increasing CPU frequency. Doing
    // this after the switch could make the core fetch instructions too quickly.
    FLASH->ACR = (FLASH->ACR & ~FLASH_ACR_LATENCY_Msk)
               | FLASH_ACR_LATENCY_3WS;
    status = waitForValue(&FLASH->ACR, FLASH_ACR_LATENCY_Msk,
                          FLASH_ACR_LATENCY_3WS);
    if (status != Status::kOk) {
        return status;
    }

    // Step 6: configure AHB/APB prescalers before selecting the fast PLL clock.
    RCC->CFGR = (RCC->CFGR & ~(RCC_CFGR_PPRE1_Msk
                             | RCC_CFGR_PPRE2_Msk
                             | RCC_CFGR_HPRE_Msk))
              | clock_config::kApb1PrescalerBits
              | clock_config::kApb2PrescalerBits
              | clock_config::kAhbPrescalerBits;

    // Step 7: convert human divider values into their register encodings.
    // M is stored as M-1 and R is encoded in steps of two; N is stored directly.
    constexpr std::uint32_t pllMBits =
        (clock_config::kPllMDiv - 1U) << RCC_PLLCFGR_PLLM_Pos;
    constexpr std::uint32_t pllNBits =
        clock_config::kPllNMul << RCC_PLLCFGR_PLLN_Pos;
    constexpr std::uint32_t pllRBits =
        ((clock_config::kPllRDiv / 2U) - 1U) << RCC_PLLCFGR_PLLR_Pos;

    // Clear every field we own, then insert HSE + M/N/R + PLLR output enable.
    // Parentheses make the mask grouping explicit and avoid precedence mistakes.
    RCC->PLLCFGR = (RCC->PLLCFGR & ~(RCC_PLLCFGR_PLLSRC_Msk
                                   | RCC_PLLCFGR_PLLM_Msk
                                   | RCC_PLLCFGR_PLLN_Msk
                                   | RCC_PLLCFGR_PLLR_Msk))
                   | RCC_PLLCFGR_PLLSRC_HSE
                   | pllMBits
                   | pllNBits
                   | pllRBits
                   | RCC_PLLCFGR_PLLREN_Msk;

    // Step 8: start the PLL and wait for lock.
    RCC->CR |= RCC_CR_PLLON_Msk;
    status = waitForSet(&RCC->CR, RCC_CR_PLLRDY_Msk);
    if (status != Status::kOk) {
        return status;
    }

    // Step 9: request PLL as SYSCLK and verify hardware reports PLL in SWS.
    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW_Msk) | RCC_CFGR_SW_PLL;
    status = waitForValue(&RCC->CFGR, RCC_CFGR_SWS_Msk, RCC_CFGR_SWS_PLL);
    if (status != Status::kOk) {
        return status;
    }

    // CMSIS and other drivers consult this global variable. It is software
    // metadata, not a register and not an independent clock measurement.
    SystemCoreClock = clock_config::kAhbClockHz;

    // Keep HSI ready as an independent fallback. Clock Security System detects
    // an HSE failure, switches the system to HSI, and raises NMI; the NMI
    // handler then enters the project safe state.
    RCC->CR |= RCC_CR_CSSON_Msk;
    return Status::kOk;
}

void disableClockOutput()
{
    // This function is safe to call even if MCO was never configured: enabling
    // the GPIO clock and clearing an already-clear selector are harmless.
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN_Msk;
    RCC->CFGR &= ~(RCC_CFGR_MCOPRE_Msk | RCC_CFGR_MCOSEL_Msk);
    // Analog mode disconnects the MCO alternate function from PA8.
    GPIOA->MODER |= GPIO_MODER_MODE8_Msk;
}

}  // namespace board
