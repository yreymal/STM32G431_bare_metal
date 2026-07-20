/*
 * Small RAII guard used to protect data shared with interrupts.
 *
 * RAII means "resource acquisition is initialization": constructing the object
 * disables interrupts and leaving its `{ ... }` scope automatically runs the
 * destructor, which restores the previous interrupt state.
 */
#pragma once

#include <cstdint>

#include "stm32g431xx.h"

// A short, nest-safe PRIMASK critical section for main/ISR shared data. It
// restores the caller's interrupt state and performs no allocation or blocking.
class CriticalSection final {
public:
    // `noexcept` promises that construction cannot throw a C++ exception.
    // The initializer list saves PRIMASK before the constructor body runs.
    CriticalSection() noexcept
        : previousPrimask_{__get_PRIMASK()}
    {
        __disable_irq();
        // DMB is a data-memory barrier: memory accesses before/after the
        // interrupt-state change cannot be reordered across this boundary.
        __DMB();
    }

    // The `~` identifies a destructor. It runs automatically at scope exit,
    // including an early `return`, which is why RAII is safer than manual pairs.
    ~CriticalSection() noexcept
    {
        __DMB();
        __set_PRIMASK(previousPrimask_);
    }

    // Copying or moving a guard would create two destructors trying to restore
    // one interrupt state. `= delete` turns such mistakes into compiler errors.
    CriticalSection(const CriticalSection&) = delete;
    CriticalSection& operator=(const CriticalSection&) = delete;
    CriticalSection(CriticalSection&&) = delete;
    CriticalSection& operator=(CriticalSection&&) = delete;

private:
    // Private data can be accessed only by this class's member functions.
    std::uint32_t previousPrimask_;
};
