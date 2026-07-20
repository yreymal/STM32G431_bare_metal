/*
 * Reusable, bounded polling helpers for memory-mapped registers.
 *
 * A hardware bit may never change when the oscillator or peripheral is broken.
 * These functions therefore count down a timeout and return a Status instead
 * of trapping the CPU forever in a `while` loop.
 */
#pragma once

#include <cstdint>

#include "status.hpp"

namespace register_utils {

inline constexpr std::uint32_t kDefaultTimeout = 0x5000U;

// `volatile const uint32_t*` means: the register value may change outside normal
// program flow, this function must reread it, and this function will not write it.
// `const` after the pointer means the pointer itself is not reassigned either.
[[nodiscard]] inline Status waitForSet(
    volatile const std::uint32_t* const reg,
    const std::uint32_t mask,
    std::uint32_t timeout = kDefaultTimeout)
{
    if (reg == nullptr) {
        return Status::kNullPointer;
    }
    if (timeout == 0U) {
        return Status::kWrongTimeout;
    }
    if (mask == 0U) {
        return Status::kWrongMask;
    }
    // `*reg` dereferences the pointer. AND with mask discards unrelated bits.
    while ((*reg & mask) == 0U) {
        if (--timeout == 0U) {
            return Status::kBitFlagIsZero;
        }
    }
    return Status::kOk;
}

[[nodiscard]] inline Status waitForValue(
    volatile const std::uint32_t* const reg,
    const std::uint32_t mask,
    const std::uint32_t value,
    std::uint32_t timeout = kDefaultTimeout)
{
    if (reg == nullptr) {
        return Status::kNullPointer;
    }
    if (timeout == 0U) {
        return Status::kWrongTimeout;
    }
    if (mask == 0U) {
        return Status::kWrongMask;
    }
    // `~mask` inverts every bit. This rejects a requested value containing bits
    // that the mask can never observe, which would otherwise force a timeout.
    if ((value & ~mask) != 0U) {
        return Status::kOutOfRange;
    }

    while ((*reg & mask) != value) {
        if (--timeout == 0U) {
            return Status::kMaskedValueMismatch;
        }
    }
    return Status::kOk;
}

[[nodiscard]] inline Status waitForClear(
    volatile const std::uint32_t* const reg,
    const std::uint32_t mask,
    std::uint32_t timeout = kDefaultTimeout)
{
    if (reg == nullptr) {
        return Status::kNullPointer;
    }
    if (timeout == 0U) {
        return Status::kWrongTimeout;
    }
    if (mask == 0U) {
        return Status::kWrongMask;
    }

    while ((*reg & mask) != 0U) {
        if (--timeout == 0U) {
            return Status::kBitIsNotZero;
        }
    }
    return Status::kOk;
}

}  // namespace register_utils
