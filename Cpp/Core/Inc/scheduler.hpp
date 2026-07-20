/*
 * Fixed-capacity cooperative scheduler interface.
 *
 * "Cooperative" means callbacks run one after another in the main thread; no
 * callback is pre-empted by another callback. Hardware interrupts can still
 * interrupt them and must follow the synchronization rules in the project.
 */
#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "status.hpp"

class Scheduler final {
public:
    // Read this declaration inside-out:
    // Callback is a pointer (*) to a function that receives void* context and
    // returns Status. The alias hides the otherwise difficult C++ syntax.
    using Callback = Status (*)(void* context);
    static constexpr std::size_t kCapacity = 25U;
    static constexpr std::uint32_t kMaximumPeriodMs = 0x7FFF'FFFFU;

    // `= default` asks the compiler to generate the simple constructor. Member
    // `{}` initializers below guarantee an empty, zeroed scheduler.
    Scheduler() = default;

    struct Statistics {
        std::uint32_t dispatchCount{};
        std::uint32_t missedReleaseCount{};
        std::uint32_t callbackFailureCount{};
    };

    [[nodiscard]] Status addTask(
        Callback callback,
        void* context,
        std::uint32_t periodMs,
        std::uint32_t registrationTimeMs);
    [[nodiscard]] Status run(std::uint32_t nowMs);

    // The final `const` promises not to change this Scheduler; `noexcept`
    // promises no exception; `&` returns the existing statistics without copy.
    [[nodiscard]] const Statistics& statistics() const noexcept;

private:
    // Users cannot create or manipulate Task directly. Keeping it private
    // protects scheduler invariants such as non-null callbacks and valid periods.
    struct Task {
        std::uint32_t lastCalledMs{};
        std::uint32_t periodMs{};
        Callback callback{};
        void* context{};
    };

    // Fixed-capacity std::array means deterministic RAM and no heap allocation.
    std::array<Task, kCapacity> tasks_{};
    std::size_t taskCount_{};
    Statistics statistics_{};
};
