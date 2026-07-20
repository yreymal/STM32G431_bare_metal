/*
 * Basic-timer TIM6 configuration and its one-second interrupt counter.
 *
 * Divider values are derived from the declared clock tree instead of repeating
 * a magic 100 MHz assumption inside this driver.
 */
#include "timer6.hpp"

#include "clock_config.hpp"
#include "seven_segment_display.hpp"
#include "stm32g431xx.h"

namespace {

// Section 1: derive timer divider values:
// 100 MHz / 10000 prescaler = 10 kHz counter;
// 10000 counter ticks / 1 Hz = 10000 ticks per update.
constexpr std::uint32_t kCounterFrequencyHz = 10'000U;
constexpr std::uint32_t kUpdateFrequencyHz = 1U;
constexpr std::uint32_t kPrescaler =
    clock_config::kApb1TimerClockHz / kCounterFrequencyHz;
constexpr std::uint32_t kPeriodTicks =
    kCounterFrequencyHz / kUpdateFrequencyHz;

// The assertions guarantee exact division and values that fit 16-bit PSC/ARR.
static_assert((clock_config::kApb1TimerClockHz % kCounterFrequencyHz) == 0U);
static_assert((kCounterFrequencyHz % kUpdateFrequencyHz) == 0U);
static_assert(kPrescaler >= 1U && kPrescaler <= 65'536U);
static_assert(kPeriodTicks >= 1U && kPeriodTicks <= 65'536U);

// Aligned 32-bit reads/writes are atomic on Cortex-M4. Volatile is required
// because these objects are modified asynchronously by the timer ISR.
volatile std::uint32_t secondTicks = 0U;
volatile std::uint32_t tickCount = 0U;

}  // namespace

namespace timer6 {

Status initialize()
{
    // Section 2: enable TIM6 and perform a dummy read. The read ensures the
    // peripheral-clock enable write has crossed the bus before TIM6 is accessed.
    RCC->APB1ENR1 |= RCC_APB1ENR1_TIM6EN_Msk;
    (void)RCC->APB1ENR1;

    // URS restricts update requests to counter overflow/underflow. Hardware
    // registers store divider-1, so 10000 is written as 9999.
    TIM6->CR1 = TIM_CR1_URS_Msk;
    TIM6->PSC = kPrescaler - 1U;
    TIM6->ARR = kPeriodTicks - 1U;
    TIM6->EGR = TIM_EGR_UG_Msk;
    // TIMx_SR flags are cleared by writing zero. Direct assignment avoids a
    // read-modify-write race with a flag arriving between the read and write.
    TIM6->SR = 0U;
    TIM6->DIER = TIM_DIER_UIE_Msk;

    // Section 3: clear a stale NVIC request before enabling the interrupt line.
    NVIC_SetPriority(TIM6_DAC_IRQn, 2U);
    NVIC_ClearPendingIRQ(TIM6_DAC_IRQn);
    NVIC_EnableIRQ(TIM6_DAC_IRQn);

    // Section 4: read back the main configuration fields.
    if ((TIM6->PSC != (kPrescaler - 1U))
        || (TIM6->ARR != (kPeriodTicks - 1U))) {
        return Status::kPeripheralFault;
    }
    return Status::kOk;
}

Status start()
{
    // Clear any event left from configuration, then set Counter Enable (CEN).
    TIM6->SR = 0U;
    TIM6->CR1 |= TIM_CR1_CEN_Msk;
    return ((TIM6->CR1 & TIM_CR1_CEN_Msk) != 0U)
         ? Status::kOk
         : Status::kPeripheralFault;
}

void stop()
{
    // Idempotent: safe even if TIM6 was never started or its clock was off.
    RCC->APB1ENR1 |= RCC_APB1ENR1_TIM6EN_Msk;
    TIM6->CR1 &= ~TIM_CR1_CEN_Msk;
    TIM6->DIER &= ~TIM_DIER_UIE_Msk;
}

std::uint32_t seconds()
{
    return secondTicks;
}

std::uint32_t ticksSeen()
{
    return tickCount;
}

Status storeSeconds(void* const context)
{
    if (context == nullptr) {
        return Status::kNullPointer;
    }

    // The physical display can represent only four decimal digits. Modulo
    // produces intentional rollover instead of clamping forever at 9999.
    // `->` accesses a struct/class member through a pointer. This one expression
    // casts the generic context and writes its number member.
    static_cast<display::State*>(context)->number =
        static_cast<std::uint16_t>(secondTicks % 10'000U);
    return Status::kOk;
}

}  // namespace timer6

extern "C" void TIM6_DAC_IRQHandler()
{
    // TIM6 shares a vector name with DAC events on this MCU family. We still
    // verify both the status flag and interrupt-enable bit before handling TIM6.
    if (((TIM6->SR & TIM_SR_UIF_Msk) != 0U)
        && ((TIM6->DIER & TIM_DIER_UIE_Msk) != 0U)) {
        TIM6->SR = 0U;
        ++secondTicks;
        ++tickCount;
    }
}
