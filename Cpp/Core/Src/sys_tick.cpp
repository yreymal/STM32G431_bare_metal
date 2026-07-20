/*
 * One-millisecond monotonic time base using the Cortex-M SysTick peripheral.
 *
 * The ISR only increments one aligned counter; scheduling remains in main
 * context and therefore does not run inside the interrupt.
 */
#include "sys_tick.hpp"

#include "stm32g431xx.h"

namespace {

// `volatile` forces every read/write to memory because the ISR changes this
// value asynchronously. Atomicity comes from aligned 32-bit Cortex-M accesses,
// not from volatile itself.
volatile std::uint32_t millisecondCounter = 0U;
static_assert(sizeof(millisecondCounter) == sizeof(std::uint32_t));

}  // namespace

namespace sys_tick {

Status initialize(const std::uint32_t hclkHz)
{
    // Section 1: validate that an exact integer number of HCLK cycles fits in a
    // 1 ms SysTick period. Modulo `%` returns the division remainder.
    if (hclkHz < kFrequencyHz) {
        return Status::kWrongTimeout;
    }
    if ((hclkHz % kFrequencyHz) != 0U) {
        // Reject a fractional divider instead of silently introducing drift in
        // the scheduler's documented millisecond time base.
        return Status::kOutOfRange;
    }

    // LOAD stores N-1 because SysTick counts all values from LOAD down to zero.
    millisecondCounter = 0U;
    const auto reload = (hclkHz / kFrequencyHz) - 1U;
    if (reload > SysTick_LOAD_RELOAD_Msk) {
        // Clamping would silently break the documented millisecond unit.
        return Status::kOutOfRange;
    }

    // Section 2: configure while stopped. CLKSOURCE selects HCLK; TICKINT asks
    // for an interrupt on zero. Writing VAL=0 clears the current counter state.
    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
    // Keep the short time-base ISR below timer and external-input priorities.
    NVIC_SetPriority(SysTick_IRQn, 7U);
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk;
    SysTick->LOAD = reload;
    SysTick->VAL = 0U;
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
    // Section 3: read back all control bits with one mask and return the result
    // using the ternary `condition ? success : failure` operator.
    constexpr std::uint32_t expectedControl = SysTick_CTRL_CLKSOURCE_Msk
                                            | SysTick_CTRL_TICKINT_Msk
                                            | SysTick_CTRL_ENABLE_Msk;
    return ((SysTick->CTRL & expectedControl) == expectedControl)
         ? Status::kOk
         : Status::kPeripheralFault;
}

std::uint32_t milliseconds() noexcept
{
    // Aligned 32-bit load/store operations are atomic on the Cortex-M4. This
    // function returns a snapshot; unsigned elapsed-time subtraction remains
    // correct across wraparound for intervals below 2^31 milliseconds.
    return millisecondCounter;
}

}  // namespace sys_tick

extern "C" void SysTick_Handler()
{
    // Unsigned overflow is defined modulo 2^32. Consumers calculate elapsed time
    // with unsigned subtraction, so the wrap itself is intentional.
    ++millisecondCounter;
}
