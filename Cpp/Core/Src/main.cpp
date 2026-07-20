/*
 * Application composition root.
 *
 * `main` owns initialization order, application state objects, task
 * registration, and the Startup -> Operational transition. Driver details stay
 * in their own modules.
 */
#include "board.hpp"
#include "clock_config.hpp"
#include "interrupts.hpp"
#include "scheduler.hpp"
#include "safety_manager.hpp"
#include "seven_segment_display.hpp"
#include "sys_tick.hpp"
#include "timer6.hpp"
#include "timer8.hpp"
#include "uart1.hpp"

#include "stm32g431xx.h"

namespace {

// An unnamed namespace gives this helper internal linkage: no other .cpp file
// can accidentally call or define the same `require` symbol.
void require(const Status status, const safety::FaultSource source)
{
    if (status != Status::kOk) {
        safety::fail(status, source);
    }
}

}  // namespace

int main()
{
    // Phase 1 — Startup supervision and system clock.
    safety::initialize();
    require(board::configureClock(), safety::FaultSource::kClock);
    // Phase 2 — Configure drivers while actuator-like TIM8 output remains gated.
    // PA5 is segment B in this application. Do not also initialize/use the
    // board status LED on PA5: two logical owners of one pin are unsafe and the
    // on-board LED circuit would additionally load the display signal.
    require(display::configureSegmentPins(), safety::FaultSource::kDisplay);
    require(interrupts::configureExternalButtons(),
            safety::FaultSource::kExternalInterrupt);
    require(timer6::initialize(), safety::FaultSource::kTimer6);

    serial::Uart1::configurePins();
    require(serial::Uart1::initialize(), safety::FaultSource::kUart1);

    require(display::configureDigitPins(), safety::FaultSource::kDisplay);
    require(sys_tick::initialize(clock_config::kSystemClockHz),
            safety::FaultSource::kSysTick);
    require(board::configureClockOutput(), safety::FaultSource::kClock);
    require(timer8::initializeOutputCompare(), safety::FaultSource::kTimer8);

    // Phase 3 — Create application-owned state on main's stack. Empty braces
    // value-initialize every field. References/pointers registered below remain
    // valid forever because main never returns and these objects keep living.
    display::State displayState{};
    display::initialize(displayState);
    interrupts::ButtonDiagnostics buttonDiagnostics{};

    Scheduler scheduler;
    // Use one common timestamp so every task's first period starts at the same
    // logical instant. `const auto` infers uint32_t and forbids reassignment.
    const auto registrationTimeMs = sys_tick::milliseconds();

    // Phase 4 — Register callbacks. `timer6::storeSeconds` decays to a function
    // pointer; `&displayState` passes the address as generic void* context.
    require(scheduler.addTask(timer6::storeSeconds, &displayState, 400U,
                              registrationTimeMs),
            safety::FaultSource::kScheduler);
    require(scheduler.addTask(display::calculateDigits, &displayState, 200U,
                              registrationTimeMs),
            safety::FaultSource::kScheduler);
    require(scheduler.addTask(display::refresh, &displayState, 4U,
                              registrationTimeMs),
            safety::FaultSource::kScheduler);
    require(scheduler.addTask(interrupts::processButtonEvents,
                              &buttonDiagnostics, 20U, registrationTimeMs),
            safety::FaultSource::kScheduler);

    // Phase 5 — Only after every registration succeeds may activity start.
    require(safety::markOperational(), safety::FaultSource::kScheduler);
    require(timer6::start(), safety::FaultSource::kTimer6);
    require(timer8::startOutputCompare(), safety::FaultSource::kTimer8);

    // Phase 6 — Cooperative super-loop. It is intentionally endless in firmware.
    while (true) {
        require(scheduler.run(sys_tick::milliseconds()),
                safety::FaultSource::kScheduler);
        // SysTick or another interrupt wakes the core. This is not a timing
        // delay or synchronization primitive; all release decisions use the
        // monotonic millisecond counter above.
        __WFI();
    }
}
