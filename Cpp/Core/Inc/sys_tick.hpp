/*
 * Public millisecond time-base interface built on the Cortex-M SysTick timer.
 */
#pragma once

#include <cstdint>

#include "status.hpp"

namespace sys_tick {

inline constexpr std::uint32_t kFrequencyHz = 1'000U;

[[nodiscard]] Status initialize(std::uint32_t hclkHz);
// `noexcept` is appropriate because reading one counter cannot fail or throw.
[[nodiscard]] std::uint32_t milliseconds() noexcept;

}  // namespace sys_tick
