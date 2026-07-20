/*
 * Public state and operations of the multiplexed four-digit display driver.
 *
 * `State` is explicitly passed to scheduled callbacks. This avoids hidden
 * global application state and makes ownership visible in main.cpp.
 */
#pragma once

#include <array>
#include <cstdint>

#include "status.hpp"
#include "display_model.hpp"

namespace display {

struct State {
    // Member `{}` syntax value-initializes everything to zero.
    std::uint16_t number{};
    std::array<std::uint8_t, 4U> digits{};
    std::uint8_t activeDigit{};
};

[[nodiscard]] Status configureSegmentPins();
[[nodiscard]] Status configureDigitPins();
// `State&` is a reference: initialize modifies the caller-owned object without
// copying it and cannot be passed null.
void initialize(State& state);
[[nodiscard]] Status calculateDigits(void* context);
[[nodiscard]] Status refresh(void* context);
void enterSafeState();

}  // namespace display
