/*
 * Central system-state and fault-management interface.
 *
 * All modules report failures through Status + FaultSource. This manager owns
 * the one-way transition into the terminal Safe state.
 */
#pragma once

#include <cstdint>

#include "status.hpp"

namespace safety {

// A scoped enum cannot be mixed accidentally with FaultSource or an integer.
enum class SystemState : std::uint8_t {
    kStartup = 0U,
    kOperational = 1U,
    kSafe = 2U,
};

enum class FaultSource : std::uint8_t {
    kNone = 0U,
    kClock,
    kDisplay,
    kExternalInterrupt,
    kTimer6,
    kUart1,
    kSysTick,
    kTimer8,
    kScheduler,
    kCpuException,
    kClockSecurity,
};

// Plain aggregate kept visible to a debugger. `{}` initializes numeric fields
// to zero; explicit enum initializers give meaningful default states.
struct FaultRecord {
    std::uint32_t magic{};
    std::uint32_t sequence{};
    std::uint32_t resetCauseFlags{};
    std::uint32_t configurableFaultStatus{};
    std::uint32_t hardFaultStatus{};
    std::uint32_t memoryFaultAddress{};
    std::uint32_t busFaultAddress{};
    Status status{Status::kOk};
    FaultSource source{FaultSource::kNone};
    SystemState state{SystemState::kStartup};
};

void initialize();
[[nodiscard]] Status markOperational();
[[nodiscard]] SystemState state();
// Returning `const volatile FaultRecord&` avoids copying and tells callers that
// asynchronous fault code may change the object but callers must not modify it.
[[nodiscard]] const volatile FaultRecord& faultRecord();

// `[[noreturn]]` documents and verifies that fail never returns to its caller.
[[noreturn]] void fail(Status status, FaultSource source);

}  // namespace safety
