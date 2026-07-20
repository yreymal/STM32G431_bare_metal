/*
 * PC-side tests for code that does not touch STM32 registers.
 *
 * This intentionally uses a tiny custom `check` function instead of a large
 * test framework, keeping the example deterministic and easy to compile with a
 * normal C++17 compiler.
 */
#include <array>
#include <cstddef>
#include <cstdint>
#include <iostream>

#include "display_model.hpp"
#include "scheduler.hpp"

namespace {

// Private test-runner state; zero means no failed checks so far.
std::uint32_t failureCount = 0U;

void check(const bool condition, const char* const description)
{
    if (!condition) {
        std::cerr << "FAILED: " << description << '\n';
        ++failureCount;
    }
}

struct CallbackContext {
    std::uint32_t callCount{};
    Status result{Status::kOk};
};

Status countingCallback(void* const opaqueContext)
{
    // This has exactly the scheduler's Status(void*) callback signature. Tests
    // use context.result to simulate both successful and failed application work.
    if (opaqueContext == nullptr) {
        return Status::kNullPointer;
    }
    auto& context = *static_cast<CallbackContext*>(opaqueContext);
    ++context.callCount;
    return context.result;
}

void testDisplayEncoding()
{
    // std::array supports value comparison, so each expected four-digit result
    // can be written directly and checked as one object.
    check(display::encodeNumber(0U)
              == display::Digits{display::kBlank, display::kBlank,
                                 display::kBlank, 0U},
          "zero uses three leading blanks");
    check(display::encodeNumber(42U)
              == display::Digits{display::kBlank, display::kBlank, 4U, 2U},
          "42 is encoded correctly");
    check(display::encodeNumber(1'000U)
              == display::Digits{1U, 0U, 0U, 0U},
          "1000 preserves interior zeroes");
    check(display::encodeNumber(9'999U)
              == display::Digits{9U, 9U, 9U, 9U},
          "upper boundary is encoded correctly");
    check(display::encodeNumber(65'535U)
              == display::Digits{9U, 9U, 9U, 9U},
          "out-of-range input saturates");
}

void testSchedulerValidationAndReleaseBehavior()
{
    // Arrange: a fresh scheduler and mutable state observed by the callback.
    Scheduler scheduler{};
    CallbackContext context{};

    check(scheduler.addTask(nullptr, &context, 10U, 100U)
              == Status::kNullPointer,
          "null callback is rejected");
    check(scheduler.addTask(countingCallback, &context, 0U, 100U)
              == Status::kOutOfRange,
          "zero period is rejected");
    check(scheduler.addTask(countingCallback, &context,
                            Scheduler::kMaximumPeriodMs + 1U, 100U)
              == Status::kOutOfRange,
          "ambiguous wraparound period is rejected");
    check(scheduler.addTask(countingCallback, &context, 10U, 100U)
              == Status::kOk,
          "valid task is accepted");

    // Act/assert at selected timestamps. At 145 ms the releases at 120 and 130
    // were missed; release 140 runs once, proving catch-up is bounded.
    check(scheduler.run(109U) == Status::kOk && context.callCount == 0U,
          "task does not run before release");
    check(scheduler.run(110U) == Status::kOk && context.callCount == 1U,
          "task runs at release");
    check(scheduler.run(145U) == Status::kOk && context.callCount == 2U,
          "late task runs once without catch-up burst");
    check(scheduler.statistics().missedReleaseCount == 2U,
          "two skipped releases are diagnosed");
    check(scheduler.statistics().dispatchCount == 2U,
          "dispatches are counted");
}

void testSchedulerFailureAndCapacity()
{
    // Fill every fixed slot, verify the next registration fails, then make the
    // first callback fail and verify Status/diagnostics propagate.
    Scheduler scheduler{};
    std::array<CallbackContext, Scheduler::kCapacity> contexts{};
    for (std::size_t index = 0U; index < contexts.size(); ++index) {
        check(scheduler.addTask(countingCallback, &contexts[index], 1U, 0U)
                  == Status::kOk,
              "task within fixed capacity is accepted");
    }
    check(scheduler.addTask(countingCallback, &contexts[0], 1U, 0U)
              == Status::kCapacityExceeded,
          "task beyond fixed capacity is rejected");

    contexts[0].result = Status::kPeripheralFault;
    check(scheduler.run(1U) == Status::kPeripheralFault,
          "callback failure is propagated");
    check(scheduler.statistics().callbackFailureCount == 1U,
          "callback failure is diagnosed");
}

void testSchedulerTimeWrap()
{
    // 0xFFFFFFF0 + 20 ms wraps to 0x00000004. Unsigned subtraction must still
    // calculate 20 elapsed milliseconds and two elapsed 10 ms periods.
    Scheduler scheduler{};
    CallbackContext context{};
    check(scheduler.addTask(countingCallback, &context, 10U,
                            0xFFFF'FFF0U) == Status::kOk,
          "wraparound test task is accepted");
    check(scheduler.run(0x0000'0004U) == Status::kOk,
          "elapsed-time subtraction survives wraparound");
    check(context.callCount == 1U,
          "wrapped task dispatches once");
    check(scheduler.statistics().missedReleaseCount == 1U,
          "wrapped skipped release is counted");
}

}  // namespace

int main()
{
    // A test executable also has one main. Returning nonzero tells CTest/CI that
    // at least one check failed.
    testDisplayEncoding();
    testSchedulerValidationAndReleaseBehavior();
    testSchedulerFailureAndCapacity();
    testSchedulerTimeWrap();

    if (failureCount != 0U) {
        std::cerr << failureCount << " test(s) failed\n";
        return 1;
    }
    std::cout << "All host unit tests passed\n";
    return 0;
}
