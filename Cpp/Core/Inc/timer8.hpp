/*
 * TIM8 output-compare driver interface.
 *
 * Advanced timer outputs have a separate Main Output Enable (MOE) safety gate,
 * so configure, start, stop, and terminal-safe operations are distinct.
 */
#pragma once

#include "status.hpp"

namespace timer8 {

[[nodiscard]] Status initializeOutputCompare();
[[nodiscard]] Status startOutputCompare();
// stopOutputCompare is a normal stop; enterSafeState also disconnects the pins.
void stopOutputCompare();
void enterSafeState();

}  // namespace timer8
