/*
 * Static USART1 driver interface for byte-oriented polling I/O.
 *
 * The implementation uses finite timeouts, so every operation that can fail
 * returns Status and is marked `[[nodiscard]]`.
 */
#pragma once

#include <cstdint>
#include "status.hpp"

namespace serial {

class Uart1 final {
public:
    // `final` says this driver is not a polymorphic base class. No virtual table
    // or inheritance is required for a single fixed hardware peripheral.
    static void configurePins();
    [[nodiscard]] static Status initialize();
    [[nodiscard]] static Status send(std::uint8_t byte);
    // A reference is used as the output parameter; it cannot be null.
    [[nodiscard]] static Status receive(std::uint8_t& byte);

    Uart1() = delete;
};

}  // namespace serial
