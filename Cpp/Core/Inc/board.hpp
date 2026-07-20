/*
 * Public interface of the board-support module.
 *
 * A header declares what other modules are allowed to call; the corresponding
 * Src/board.cpp file contains the implementation. `#pragma once` below tells
 * the compiler to include this header only once per translation unit.
 */
#pragma once

#include <cstdint>

#include "status.hpp"

namespace board {

// `[[nodiscard]]` asks the compiler to warn if the caller ignores the Status.
// Clock configuration can fail, so main must check the returned value.
[[nodiscard]] Status configureClock();
[[nodiscard]] Status configureClockOutput();

// Safe-state operations are `void`: they are best-effort, non-blocking actions
// used after a fault, when there is no meaningful higher-level recovery path.
void disableClockOutput();

}  // namespace board: prevents board names colliding with other modules
