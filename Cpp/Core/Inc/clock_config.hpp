/*
 * Compile-time clock configuration and validation.
 *
 * `inline constexpr` values occupy no mutable runtime state: the compiler can
 * calculate them while building the program. The `static_assert` statements at
 * the bottom stop compilation when an impossible clock plan is entered.
 */
#pragma once

#include <cstdint>

#include "stm32g431xx.h"

namespace clock_config {

// Section 1: physical source. Apostrophes are digit separators only; the value
// is exactly 24000000. The U suffix makes the integer unsigned.
inline constexpr std::uint32_t kHseHz = 24'000'000U;
// The supplied board schematic shows fitted 24 MHz crystal X3 connected to
// PF0/PF1 through SB26/SB25. The alternative ST-LINK MCO links are DNF, so the
// oscillator must run in crystal mode and HSE bypass must remain disabled.
inline constexpr bool kHseBypass = false;
inline constexpr std::uint32_t kPllMDiv = 3U;
inline constexpr std::uint32_t kPllNMul = 25U;
inline constexpr std::uint32_t kPllRDiv = 2U;

// Section 2: bus prescalers. A divider of one means HCLK, PCLK1 and PCLK2 all
// run at the resulting 100 MHz in this project.
inline constexpr std::uint32_t kAhbPrescaler = 1U;
inline constexpr std::uint32_t kApb1Prescaler = 1U;
inline constexpr std::uint32_t kApb2Prescaler = 1U;

// Section 3: frequency equations evaluated by the compiler:
// 24 MHz / 3 = 8 MHz PLL input; 8 MHz * 25 = 200 MHz VCO;
// 200 MHz / 2 = 100 MHz SYSCLK.
inline constexpr std::uint32_t kPllInputHz = kHseHz / kPllMDiv;
inline constexpr std::uint32_t kVcoHz = kPllInputHz * kPllNMul;
inline constexpr std::uint32_t kSystemClockHz = kVcoHz / kPllRDiv;
inline constexpr std::uint32_t kAhbClockHz = kSystemClockHz / kAhbPrescaler;
inline constexpr std::uint32_t kApb1ClockHz = kAhbClockHz / kApb1Prescaler;
inline constexpr std::uint32_t kApb2ClockHz = kAhbClockHz / kApb2Prescaler;

// STM32 timers receive twice PCLK when an APB prescaler is greater than one.
// The ternary operator is `condition ? value_if_true : value_if_false`.
inline constexpr std::uint32_t kApb1TimerClockHz =
    (kApb1Prescaler == 1U) ? kApb1ClockHz : (2U * kApb1ClockHz);
inline constexpr std::uint32_t kApb2TimerClockHz =
    (kApb2Prescaler == 1U) ? kApb2ClockHz : (2U * kApb2ClockHz);

// Section 4: bit patterns written to RCC->CFGR. Frequencies above describe the
// math; these CMSIS constants describe the matching hardware encoding.
inline constexpr std::uint32_t kAhbPrescalerBits = RCC_CFGR_HPRE_DIV1;
inline constexpr std::uint32_t kApb1PrescalerBits = RCC_CFGR_PPRE1_DIV1;
inline constexpr std::uint32_t kApb2PrescalerBits = RCC_CFGR_PPRE2_DIV1;

inline constexpr std::uint32_t kMcoDivider = 16U;
inline constexpr std::uint32_t kMcoOutputHz = kSystemClockHz / kMcoDivider;

// Section 5: compile-time safety net. `static_assert(false)` is a build error,
// so a bad divider cannot silently become a faulty runtime clock.
static_assert(kPllMDiv >= 1U && kPllMDiv <= 16U);
static_assert(kPllNMul >= 8U && kPllNMul <= 127U);
static_assert(kPllRDiv == 2U || kPllRDiv == 4U
              || kPllRDiv == 6U || kPllRDiv == 8U);
static_assert((kHseHz % kPllMDiv) == 0U);
static_assert(kPllInputHz >= 2'660'000U && kPllInputHz <= 16'000'000U);
static_assert(kVcoHz >= 96'000'000U && kVcoHz <= 344'000'000U);
static_assert(kSystemClockHz == 100'000'000U);
static_assert(kSystemClockHz <= 170'000'000U);
static_assert(kMcoOutputHz == 6'250'000U);

}  // namespace clock_config
