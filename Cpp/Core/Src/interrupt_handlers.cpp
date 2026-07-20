/*
 * Cortex-M exception handlers referenced by the startup vector table.
 *
 * `extern "C"` is essential: it prevents C++ name mangling so the linker can
 * match these exact names with the weak handlers in the STM32 startup file.
 */
#include "safety_manager.hpp"
#include "stm32g431xx.h"

extern "C" {

// NMI cannot be masked by normal interrupt control. In this project its expected
// source is RCC Clock Security System, so it receives a clock-specific source.
[[noreturn]] void NMI_Handler()
{
    safety::fail(Status::kClockError, safety::FaultSource::kClockSecurity);
}

[[noreturn]] void HardFault_Handler()
{
    safety::fail(Status::kPeripheralFault, safety::FaultSource::kCpuException);
}

[[noreturn]] void MemManage_Handler()
{
    safety::fail(Status::kPeripheralFault, safety::FaultSource::kCpuException);
}

[[noreturn]] void BusFault_Handler()
{
    safety::fail(Status::kPeripheralFault, safety::FaultSource::kCpuException);
}

[[noreturn]] void UsageFault_Handler()
{
    safety::fail(Status::kPeripheralFault, safety::FaultSource::kCpuException);
}

void SVC_Handler()
{
    // Reserved for an RTOS/system-call mechanism. Empty because this project is
    // bare metal and never deliberately executes SVC.
}

void DebugMon_Handler()
{
    // Reserved for Cortex-M debug monitor events; not enabled by this project.
}

void PendSV_Handler()
{
    // RTOS kernels commonly context-switch here. The cooperative scheduler has
    // one execution stack and therefore needs no PendSV context switch.
}

}  // extern "C"
