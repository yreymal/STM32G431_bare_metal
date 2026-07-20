/*
 * EXTI configuration, minimal button ISRs, and deferred debounce processing.
 *
 * The important architectural rule is visible here: interrupt context captures
 * an event; normal scheduler context decides what that event means.
 */
#include "interrupts.hpp"

#include <cstdint>
#include <limits>

#include "critical_section.hpp"
#include "sys_tick.hpp"
#include "stm32g431xx.h"

namespace {

// ISRs only set these flags. The scheduled event processor performs the real
// work, keeping interrupt latency short and deterministic.
volatile std::uint32_t pendingButtons = 0U;
// Shift one unsigned bit into a unique position for each event. OR can combine
// both events in the same word without losing their identity.
constexpr std::uint32_t kButton0Event = 1UL << 0U;
constexpr std::uint32_t kButton1Event = 1UL << 1U;
constexpr std::uint32_t kDebounceMs = 50U;

void saturatingIncrement(std::uint32_t& value)
{
    if (value != std::numeric_limits<std::uint32_t>::max()) {
        ++value;
    }
}

}  // namespace

namespace interrupts {

Status configureExternalButtons()
{
    // Section 1: clocks are required before GPIO/SYSCFG register writes work.
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN_Msk;
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN_Msk;

    // Section 2: PA0/PA1 input mode (00) with pull-ups (01). An unpressed input
    // reads high; a button connected to ground creates the falling edge.
    GPIOA->MODER &= ~(GPIO_MODER_MODE0_Msk | GPIO_MODER_MODE1_Msk);
    GPIOA->PUPDR = (GPIOA->PUPDR
                    & ~(GPIO_PUPDR_PUPD0_Msk | GPIO_PUPDR_PUPD1_Msk))
                   | GPIO_PUPDR_PUPD0_0
                   | GPIO_PUPDR_PUPD1_0;

    // Section 3: route EXTI lines 0/1 to port A, mask them while configuring,
    // disable rising edges, enable falling edges, clear stale pending requests.
    SYSCFG->EXTICR[0] &= ~(SYSCFG_EXTICR1_EXTI0_Msk
                         | SYSCFG_EXTICR1_EXTI1_Msk);
    EXTI->IMR1 &= ~(EXTI_IMR1_IM0_Msk | EXTI_IMR1_IM1_Msk);
    EXTI->RTSR1 &= ~(EXTI_RTSR1_RT0_Msk | EXTI_RTSR1_RT1_Msk);
    EXTI->FTSR1 |= EXTI_FTSR1_FT0_Msk | EXTI_FTSR1_FT1_Msk;
    // EXTI PR1 is write-one-to-clear. Clear stale edges before unmasking.
    EXTI->PR1 = EXTI_PR1_PIF0_Msk | EXTI_PR1_PIF1_Msk;
    EXTI->IMR1 |= EXTI_IMR1_IM0_Msk | EXTI_IMR1_IM1_Msk;

    // Section 4: configure Cortex-M NVIC. A lower numeric value is a higher
    // interrupt priority; 6 keeps button edges below the TIM6 priority of 2.
    NVIC_SetPriority(EXTI0_IRQn, 6U);
    // Both handlers update the same flag word. Giving them equal pre-emption
    // priority prevents one read-modify-write operation from interrupting the
    // other and losing an event bit.
    NVIC_SetPriority(EXTI1_IRQn, 6U);
    NVIC_ClearPendingIRQ(EXTI0_IRQn);
    NVIC_ClearPendingIRQ(EXTI1_IRQn);
    NVIC_EnableIRQ(EXTI0_IRQn);
    NVIC_EnableIRQ(EXTI1_IRQn);
    return Status::kOk;
}

Status processButtonEvents(void* const context)
{
    // Section 1: validate the scheduler's untyped pointer before conversion.
    if (context == nullptr) {
        return Status::kNullPointer;
    }
    // static_cast says: "context was originally a ButtonDiagnostics address".
    // Dereference with * and bind a reference so later code uses a normal name.
    auto& diagnostics = *static_cast<ButtonDiagnostics*>(context);

    std::uint32_t events = 0U;
    {
        // The extra braces create a deliberately short lifetime. Interrupts are
        // disabled only for the two shared accesses, then restored immediately.
        const CriticalSection criticalSection{};
        events = pendingButtons;
        pendingButtons = 0U;
    }

    const std::uint32_t now = sys_tick::milliseconds();
    // Section 3: unsigned subtraction remains correct when the millisecond
    // counter wraps from 0xFFFFFFFF to zero (within the documented interval).
    const bool debounceExpired = !diagnostics.hasAcceptedEvent
                               || static_cast<std::uint32_t>(
                                      now - diagnostics.lastAcceptedMs)
                                      >= kDebounceMs;
    if ((events != 0U) && debounceExpired) {
        // This driver records input only. It never writes application/display
        // outputs owned by another module. The application may consume these
        // diagnostics later through an explicitly defined control interface.
        diagnostics.lastAcceptedMask = events;
        saturatingIncrement(diagnostics.acceptedEventCount);
        diagnostics.lastAcceptedMs = now;
        diagnostics.hasAcceptedEvent = true;
    } else if (events != 0U) {
        saturatingIncrement(diagnostics.rejectedByDebounceCount);
    }
    return Status::kOk;
}

}  // namespace interrupts

extern "C" void EXTI0_IRQHandler()
{
    // Check the peripheral flag because an IRQ entry can be pending/stale. PR1
    // is W1C: direct assignment of one clears this flag without clearing others.
    if ((EXTI->PR1 & EXTI_PR1_PIF0_Msk) != 0U) {
        // Clear the peripheral request immediately, record the event, return.
        EXTI->PR1 = EXTI_PR1_PIF0_Msk;
        pendingButtons |= kButton0Event;
    }
}

extern "C" void EXTI1_IRQHandler()
{
    if ((EXTI->PR1 & EXTI_PR1_PIF1_Msk) != 0U) {
        EXTI->PR1 = EXTI_PR1_PIF1_Msk;
        pendingButtons |= kButton1Event;
    }
}
