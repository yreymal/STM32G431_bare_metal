/*
 * Hardware-independent number-to-digits conversion.
 *
 * Keeping this algorithm in a header-only pure function makes it easy to test
 * on a PC without pretending that STM32 GPIO registers exist there.
 */
#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace display {

// 0xFF is outside the valid digit range 0..9, so it is an unambiguous sentinel
// meaning "do not illuminate a digit".
inline constexpr std::uint8_t kBlank = 0xFFU;
inline constexpr std::uint16_t kMaximumDisplayNumber = 9'999U;

// `using` creates a readable alias. Digits means exactly four uint8_t values;
// unlike a raw C array, std::array knows its size and supports bounds reasoning.
using Digits = std::array<std::uint8_t, 4U>;

// Convert a value to four decimal digits. Values outside the physical display
// range are saturated. Leading zeroes are represented by kBlank, while zero is
// represented by "   0". The function is pure and suitable for host tests.
[[nodiscard]] constexpr Digits encodeNumber(std::uint16_t number) noexcept
{
    // Saturation gives defined behavior even if memory/input contains 10000+.
    if (number > kMaximumDisplayNumber) {
        number = kMaximumDisplayNumber;
    }

    // Braces initialize all four elements. `static_cast` makes the intentional
    // narrowing from uint16_t arithmetic to an individual uint8_t digit explicit.
    Digits digits{
        static_cast<std::uint8_t>(number / 1'000U),
        static_cast<std::uint8_t>((number / 100U) % 10U),
        static_cast<std::uint8_t>((number / 10U) % 10U),
        static_cast<std::uint8_t>(number % 10U),
    };

    // Stop before the final element so number zero becomes "   0", not blank.
    for (std::size_t index = 0U; index < (digits.size() - 1U); ++index) {
        if (digits[index] != 0U) {
            break;
        }
        digits[index] = kBlank;
    }
    return digits;
}

// Because encodeNumber is constexpr, these are tiny tests executed by the
// compiler itself. They add to, but do not replace, the host unit tests.
static_assert(encodeNumber(0U)[3] == 0U);
static_assert(encodeNumber(42U)[2] == 4U);
static_assert(encodeNumber(9'999U)[0] == 9U);

}  // namespace display
