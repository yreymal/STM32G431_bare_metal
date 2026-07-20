/*
 * Public interface for the two external button interrupts.
 *
 * The ISR only records raw edges. `processButtonEvents` runs later in normal
 * scheduler context and applies debounce logic.
 */
#pragma once

#include <cstdint>

#include "status.hpp"

namespace interrupts {

// `{}` after each member requests zero/false initialization. The same object
// contains both externally useful counters and the state required by debounce.
struct ButtonDiagnostics {
    std::uint32_t acceptedEventCount{};
    std::uint32_t rejectedByDebounceCount{};
    std::uint32_t lastAcceptedMask{};
    std::uint32_t lastAcceptedMs{};
    bool hasAcceptedEvent{};
};

// Configure PA0/PA1, EXTI routing, edge detection, and NVIC priorities.
[[nodiscard]] Status configureExternalButtons();
// Process ISR-recorded button events in normal scheduler context. A 20 ms
// scheduler period also serves as simple switch debounce/coalescing.
// `void*` is the scheduler's generic context pointer. The implementation checks
// it for null, then casts it back to ButtonDiagnostics*.
[[nodiscard]] Status processButtonEvents(void* context);

}  // namespace interrupts
