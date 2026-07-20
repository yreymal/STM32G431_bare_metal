/*
 * Advanced-timer TIM8 output-compare configuration for PC6 and PC7.
 *
 * The counter is configured while MOE is cleared. Output pins become active
 * only in `startOutputCompare`, after the system enters Operational state.
 */
#include "timer8.hpp"

#include <cstdint>

#include "stm32g431xx.h"

namespace {

// GPIO pin numbers are used to calculate register field positions. Every GPIO
// MODER field is two bits; every AFR field is four bits.
constexpr std::uint32_t kChannel1Pin = 6U;
constexpr std::uint32_t kChannel2Pin = 7U;
constexpr std::uint32_t kAlternateFunction = 4U;

// Output-compare demo configuration. With the 100 MHz APB2 timer clock this
// gives a 1 MHz counter and a 20 kHz repetition period.
constexpr std::uint32_t kPrescaler = 100U;
constexpr std::uint32_t kPeriodTicks = 50U;
constexpr std::uint32_t kCompare1 = 2U;
constexpr std::uint32_t kCompare2 = 5U;

static_assert(kPrescaler > 0U);
static_assert(kPeriodTicks > 0U);
static_assert(kCompare1 < kPeriodTicks);
static_assert(kCompare2 < kPeriodTicks);
static_assert(kPrescaler <= 65'536U);
static_assert(kPeriodTicks <= 65'536U);

}  // namespace

namespace timer8 {

Status initializeOutputCompare()
{
    // Section 1: enable GPIOC and TIM8 clocks. Dummy reads provide the required
    // bus synchronization before the newly clocked blocks are accessed.
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOCEN;
    RCC->APB2ENR |= RCC_APB2ENR_TIM8EN;
    (void)RCC->AHB2ENR;
    (void)RCC->APB2ENR;

    // Section 2: configure PC6/PC7 as alternate function (binary 10), push-pull,
    // medium speed, no pull. One calculated mask handles both pins together.
    constexpr std::uint32_t modeMask = (3UL << (kChannel1Pin * 2U))
                                     | (3UL << (kChannel2Pin * 2U));
    GPIOC->MODER = (GPIOC->MODER & ~modeMask)
                 | (2UL << (kChannel1Pin * 2U))
                 | (2UL << (kChannel2Pin * 2U));
    GPIOC->OTYPER &= ~((1UL << kChannel1Pin) | (1UL << kChannel2Pin));
    GPIOC->OSPEEDR = (GPIOC->OSPEEDR & ~modeMask)
                    | (2UL << (kChannel1Pin * 2U))
                    | (2UL << (kChannel2Pin * 2U));
    GPIOC->PUPDR &= ~modeMask;

    // AFR[0] covers pins 0..7. Each four-bit field receives AF4 for TIM8.
    constexpr std::uint32_t afMask = (0xFUL << (kChannel1Pin * 4U))
                                   | (0xFUL << (kChannel2Pin * 4U));
    GPIOC->AFR[0] = (GPIOC->AFR[0] & ~afMask)
                  | (kAlternateFunction << (kChannel1Pin * 4U))
                  | (kAlternateFunction << (kChannel2Pin * 4U));

    // Section 3: stop/reset all timer modes owned by this driver. Writing known
    // complete values makes the result independent of bootloader/debug residue.
    TIM8->CR1 = 0U;
    TIM8->CR2 = 0U;
    TIM8->SMCR = 0U;
    TIM8->DIER = 0U;
    TIM8->CCMR1 = 0U;
    TIM8->CCER = 0U;
    TIM8->BDTR = 0U;

    // Section 4: program the time base and two compare instants.
    TIM8->PSC = kPrescaler - 1U;
    TIM8->ARR = kPeriodTicks - 1U;
    TIM8->CNT = 0U;
    TIM8->CCR1 = kCompare1;
    TIM8->CCR2 = kCompare2;

    // OCxM=011 is toggle-on-match output compare mode for both channels.
    TIM8->CCMR1 = TIM_CCMR1_OC1M_0
                | TIM_CCMR1_OC1M_1
                | TIM_CCMR1_OC2M_0
                | TIM_CCMR1_OC2M_1;
    TIM8->CCER = TIM_CCER_CC1E_Msk | TIM_CCER_CC2E_Msk;
    // Keep the advanced-timer main output disabled until the system has
    // completed initialization and explicitly enters Operational state.
    TIM8->BDTR = 0U;
    TIM8->EGR = TIM_EGR_UG_Msk;
    TIM8->SR = 0U;

    // Section 5: basic register readback before reporting successful init.
    if ((TIM8->PSC != (kPrescaler - 1U))
        || (TIM8->ARR != (kPeriodTicks - 1U))
        || (TIM8->CCR1 != kCompare1)
        || (TIM8->CCR2 != kCompare2)) {
        return Status::kPeripheralFault;
    }
    return Status::kOk;
}

Status startOutputCompare()
{
    // Restart from a deterministic phase, transfer prescaler/ARR with UG, clear
    // update status, open the advanced-timer MOE gate, then start the counter.
    TIM8->CNT = 0U;
    TIM8->EGR = TIM_EGR_UG_Msk;
    TIM8->SR = 0U;
    TIM8->BDTR |= TIM_BDTR_MOE_Msk;
    TIM8->CR1 |= TIM_CR1_CEN_Msk;

    constexpr std::uint32_t requiredBits = TIM_BDTR_MOE_Msk;
    return (((TIM8->BDTR & requiredBits) == requiredBits)
            && ((TIM8->CR1 & TIM_CR1_CEN_Msk) != 0U))
         ? Status::kOk
         : Status::kPeripheralFault;
}

void stopOutputCompare()
{
    // Clear MOE first so external waveforms stop before the counter stops.
    RCC->APB2ENR |= RCC_APB2ENR_TIM8EN_Msk;
    TIM8->BDTR &= ~TIM_BDTR_MOE_Msk;
    TIM8->CR1 &= ~TIM_CR1_CEN_Msk;
}

void enterSafeState()
{
    // Analog GPIO mode (binary 11) disconnects the alternate-function driver.
    stopOutputCompare();
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOCEN_Msk;
    // Disconnect both timer channels from their pins after MOE is cleared.
    constexpr std::uint32_t modeMask = (3UL << (kChannel1Pin * 2U))
                                     | (3UL << (kChannel2Pin * 2U));
    GPIOC->MODER |= modeMask;
}

}  // namespace timer8
