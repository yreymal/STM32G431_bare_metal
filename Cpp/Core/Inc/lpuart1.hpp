/*
 * Static driver interface for LPUART1.
 *
 * Every operation is static because the STM32 contains one LPUART1 hardware
 * instance. The class cannot be constructed, copied, or stored accidentally.
 */
#pragma once

#include <cstdint>
#include "status.hpp"

namespace serial {

class LowPowerUart1 final {
public:
    // Static member functions are called as LowPowerUart1::initialize(); no
    // object or hidden `this` pointer is involved.
    static void configurePins();
    [[nodiscard]] static Status initialize();
    [[nodiscard]] static Status send(std::uint8_t byte);
    // `&` makes byte an output reference: receive writes directly into the
    // caller's variable after a valid byte arrives.
    [[nodiscard]] static Status receive(std::uint8_t& byte);

    // There is only one peripheral instance, so object construction is banned.
    LowPowerUart1() = delete;
};

}  // namespace serial
