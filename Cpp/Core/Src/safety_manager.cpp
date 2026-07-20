/*
 * Centralized system state, diagnostic snapshot, and terminal fault reaction.
 *
 * This is intentionally small: a safety reaction should be understandable,
 * bounded, and independent of the cooperative scheduler.
 */
#include "safety_manager.hpp"

#include "board.hpp"
#include "seven_segment_display.hpp"
#include "timer6.hpp"
#include "timer8.hpp"
#include "stm32g431xx.h"

namespace {

// Internal linkage keeps these objects private to this translation unit.
// `volatile` is used because fault/exception context can update them outside the
// normal main flow and we want every debugger-visible access to reach memory.
constexpr std::uint32_t kFaultRecordMagic = 0x5341'4645U;  // "SAFE"
volatile safety::SystemState systemState = safety::SystemState::kStartup;
volatile safety::FaultRecord lastFault{};

}  // namespace

namespace safety {

void initialize()
{
    // Section 1: establish a known software state and collect the RCC reset-cause
    // flags before clearing them. The magic value helps a debugger recognize a
    // valid record rather than random/old memory.
    systemState = SystemState::kStartup;
    lastFault.magic = kFaultRecordMagic;
    lastFault.sequence = 0U;
    lastFault.resetCauseFlags = RCC->CSR;
    lastFault.configurableFaultStatus = 0U;
    lastFault.hardFaultStatus = 0U;
    lastFault.memoryFaultAddress = 0U;
    lastFault.busFaultAddress = 0U;
    lastFault.status = Status::kOk;
    lastFault.source = FaultSource::kNone;
    lastFault.state = SystemState::kStartup;

    // Preserve the sampled reset cause above, then clear hardware reset flags
    // so a subsequent reset can be distinguished.
    RCC->CSR |= RCC_CSR_RMVF_Msk;
}

Status markOperational()
{
    // This guard makes the state transition one-way. Calling it twice, or trying
    // to leave Safe state without a reset, is a diagnosed programming error.
    if (systemState != SystemState::kStartup) {
        return Status::kInvalidState;
    }
    systemState = SystemState::kOperational;
    return Status::kOk;
}

SystemState state()
{
    // Returning the small enum by value creates a coherent snapshot.
    return systemState;
}

const volatile FaultRecord& faultRecord()
{
    // Returning a reference avoids copying several hardware diagnostic fields.
    return lastFault;
}

[[noreturn]] void fail(const Status status, const FaultSource source)
{
    // Section 2: prevent ordinary interrupts from running while outputs and the
    // diagnostic record are being placed in their terminal condition.
    __disable_irq();
    systemState = SystemState::kSafe;

    // Saturate the sequence at UINT32_MAX. Unsigned wrap is defined by C++, but
    // wrapping to zero would make diagnostics misleading.
    lastFault.magic = kFaultRecordMagic;
    if (lastFault.sequence != 0xFFFF'FFFFU) {
        lastFault.sequence = lastFault.sequence + 1U;
    }
    lastFault.status = status;
    lastFault.source = source;
    lastFault.state = SystemState::kSafe;
    // CFSR/HFSR explain CPU faults; MMFAR/BFAR contain fault addresses when the
    // corresponding validity bits say they are meaningful.
    lastFault.configurableFaultStatus = SCB->CFSR;
    lastFault.hardFaultStatus = SCB->HFSR;
    lastFault.memoryFaultAddress = SCB->MMFAR;
    lastFault.busFaultAddress = SCB->BFAR;

    // Each hardware owner supplies an idempotent, non-blocking safe action.
    // TIM8 MOE is cleared before its pins are disconnected. Display digit
    // selects are disabled before segment pins are touched.
    timer8::enterSafeState();
    timer6::stop();
    display::enterSafeState();
    board::disableClockOutput();

    // Section 4: barriers ensure previous register/memory writes complete before
    // entering the final loop. NOP performs no operation but prevents returning.
    __DSB();
    __ISB();
    while (true) {
        __NOP();
    }
}

}  // namespace safety
