/*
 * C++17 implementation of the STM32G4 CMSIS system contract.
 *
 * CMSIS register headers are already valid in both C and C++. The symbols in
 * this file retain C linkage because the startup assembly and CMSIS headers use
 * their unmangled ABI names.
 */
#include <cstdint>

#include "stm32g4xx.h"

namespace {

constexpr std::uint32_t kHsiFrequencyHz = 16'000'000U;
constexpr std::uint32_t kHseFrequencyHz = 24'000'000U;

}  // namespace

extern "C" {

std::uint32_t SystemCoreClock = kHsiFrequencyHz;

const std::uint8_t AHBPrescTable[16] = {
    0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U,
    1U, 2U, 3U, 4U, 6U, 7U, 8U, 9U};

const std::uint8_t APBPrescTable[8] = {
    0U, 0U, 0U, 0U, 1U, 2U, 3U, 4U};

void SystemInit()
{
#if (__FPU_PRESENT == 1U) && (__FPU_USED == 1U)
    // Grant full access to CP10 and CP11 before any floating-point instruction.
    SCB->CPACR |= (3UL << (10UL * 2UL)) | (3UL << (11UL * 2UL));
#endif
}

void SystemCoreClockUpdate()
{
    std::uint32_t systemClockHz = SystemCoreClock;

    switch (RCC->CFGR & RCC_CFGR_SWS_Msk) {
    case RCC_CFGR_SWS_HSI:
        systemClockHz = kHsiFrequencyHz;
        break;

    case RCC_CFGR_SWS_HSE:
        systemClockHz = kHseFrequencyHz;
        break;

    case RCC_CFGR_SWS_PLL: {
        const std::uint32_t pllSource =
            RCC->PLLCFGR & RCC_PLLCFGR_PLLSRC_Msk;
        const std::uint32_t pllInputHz =
            (pllSource == RCC_PLLCFGR_PLLSRC_HSI)
                ? kHsiFrequencyHz
                : kHseFrequencyHz;
        const std::uint32_t pllM =
            ((RCC->PLLCFGR & RCC_PLLCFGR_PLLM_Msk)
             >> RCC_PLLCFGR_PLLM_Pos) + 1U;
        const std::uint32_t pllN =
            (RCC->PLLCFGR & RCC_PLLCFGR_PLLN_Msk)
            >> RCC_PLLCFGR_PLLN_Pos;
        const std::uint32_t pllR =
            ((((RCC->PLLCFGR & RCC_PLLCFGR_PLLR_Msk)
               >> RCC_PLLCFGR_PLLR_Pos) + 1U) * 2U);

        systemClockHz = ((pllInputHz / pllM) * pllN) / pllR;
        break;
    }

    default:
        // A transient or reserved SWS value cannot be converted reliably.
        return;
    }

    const std::uint32_t ahbIndex =
        (RCC->CFGR & RCC_CFGR_HPRE_Msk) >> RCC_CFGR_HPRE_Pos;
    SystemCoreClock = systemClockHz >> AHBPrescTable[ahbIndex];
}

}  // extern "C"
