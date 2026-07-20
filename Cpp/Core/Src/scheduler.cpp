/*
 * Hardware-independent cooperative scheduler implementation.
 *
 * The caller supplies `nowMs`, which removes an STM32 dependency and lets the
 * exact release behavior be unit tested on a PC.
 */
#include "scheduler.hpp"

#include <limits>

namespace {

// Add without allowing a diagnostic counter to wrap from 0xFFFFFFFF to zero.
// std::numeric_limits avoids hard-coding the maximum value of uint32_t.
void saturatingAdd(std::uint32_t& destination, const std::uint32_t increment)
{
    const auto maximum = std::numeric_limits<std::uint32_t>::max();
    // Ternary expression: choose maximum on overflow risk, otherwise add.
    destination = (increment > (maximum - destination))
                ? maximum
                : static_cast<std::uint32_t>(destination + increment);
}

}  // namespace

Status Scheduler::addTask(
    const Callback callback,
    void* const context,
    const std::uint32_t periodMs,
    const std::uint32_t registrationTimeMs)
{
    // Section 1: validate all invariants before changing the task array.
    if (callback == nullptr) {
        return Status::kNullPointer;
    }
    if ((periodMs == 0U) || (periodMs > kMaximumPeriodMs)) {
        return Status::kOutOfRange;
    }
    if (taskCount_ >= tasks_.size()) {
        return Status::kCapacityExceeded;
    }

    // Aggregate initialization fills Task members in declaration order. Only
    // after the complete task is stored do we publish it by incrementing count.
    tasks_[taskCount_] = Task{registrationTimeMs, periodMs, callback, context};
    ++taskCount_;
    return Status::kOk;
}

Status Scheduler::run(const std::uint32_t nowMs)
{
    // Section 2: the for-loop is statically bounded by kCapacity. `auto&` below
    // is a reference to the existing task, not a copy, so lastCalledMs can change.
    for (std::size_t index = 0U; index < taskCount_; ++index) {
        auto& task = tasks_[index];
        // Unsigned subtraction deliberately handles the 32-bit millisecond wrap.
        const auto elapsedMs = static_cast<std::uint32_t>(
            nowMs - task.lastCalledMs);
        if (elapsedMs >= task.periodMs) {
            // Division says how many releases elapsed, including the one we run.
            const auto elapsedPeriods = elapsedMs / task.periodMs;
            if (elapsedPeriods > 1U) {
                saturatingAdd(statistics_.missedReleaseCount,
                              elapsedPeriods - 1U);
            }

            // Skip missed releases and call each task no more than once per
            // scheduler pass. This bounds dispatch work after a long stall and
            // prevents a burst of catch-up calls from starving other tasks.
            task.lastCalledMs += elapsedPeriods * task.periodMs;
            saturatingAdd(statistics_.dispatchCount, 1U);

            // Dereferencing/calling a function pointer looks like a normal call.
            // The context restores the type-specific state chosen at registration.
            const auto status = task.callback(task.context);
            if (status != Status::kOk) {
                saturatingAdd(statistics_.callbackFailureCount, 1U);
                return status;
            }
        }
    }
    return Status::kOk;
}

const Scheduler::Statistics& Scheduler::statistics() const noexcept
{
    // The caller receives read-only access to the existing member object.
    return statistics_;
}
