/*
 * Common error type used by every failure-capable project API.
 *
 * `enum class` prevents accidental conversion to/from ordinary integers. The
 * explicit uint8_t base keeps the representation small and predictable.
 */
#pragma once

#include <cstdint>

enum class Status : std::uint8_t {
    // Explicit hexadecimal codes make debugger inspection and logging stable.
    kOk = 0x00U,
    kWrongTimeout = 0x01U,
    kClockError = 0x02U,
    kBitFlagIsZero = 0x03U,
    kBitIsNotZero = 0x04U,
    kMaskedValueMismatch = 0x05U,
    kWrongMask = 0x06U,
    kNullPointer = 0x07U,
    kOutOfRange = 0x08U,
    kPeripheralTimeout = 0x09U,
    kInvalidState = 0x0AU,
    kCapacityExceeded = 0x0BU,
    kPeripheralFault = 0x0CU,
};

// Protect the intended one-byte representation from an unexpected toolchain.
static_assert(sizeof(Status) == sizeof(std::uint8_t));
