/*
 * GPIO driver for a four-digit common-anode seven-segment display.
 *
 * Multiplexing means only one physical digit is enabled at a time. The refresh
 * task changes digits quickly enough that human vision sees a stable number.
 */
#include "seven_segment_display.hpp"

#include <array>

#include "stm32g431xx.h"

namespace {

// Section 1: logical segments mapped to physical GPIOA output bits. The names
// use GPIO masks rather than plain pin numbers so they can be ORed directly.
constexpr std::uint32_t kSegmentA = GPIO_ODR_OD4_Msk;
constexpr std::uint32_t kSegmentB = GPIO_ODR_OD5_Msk;
constexpr std::uint32_t kSegmentC = GPIO_ODR_OD6_Msk;
constexpr std::uint32_t kSegmentD = GPIO_ODR_OD7_Msk;
constexpr std::uint32_t kSegmentE = GPIO_ODR_OD11_Msk;
constexpr std::uint32_t kSegmentF = GPIO_ODR_OD9_Msk;
constexpr std::uint32_t kSegmentG = GPIO_ODR_OD10_Msk;
constexpr std::uint32_t kAllSegments = kSegmentA | kSegmentB | kSegmentC
                                     | kSegmentD | kSegmentE | kSegmentF
                                     | kSegmentG;

// Section 2: lookup table for common-anode polarity, where HIGH means OFF.
// Each entry stores the segments that must remain high/off. For example, digit
// 0 leaves only G high; digit 8 stores zero because no segment is off.
constexpr std::array<std::uint32_t, 10U> kDigitPatterns{
    kSegmentG,
    kAllSegments & ~(kSegmentB | kSegmentC),
    kSegmentF | kSegmentC,
    kSegmentE | kSegmentF,
    kSegmentA | kSegmentD | kSegmentE,
    kSegmentB | kSegmentE,
    kSegmentB,
    kAllSegments & ~(kSegmentA | kSegmentB | kSegmentC),
    0U,
    kSegmentE,
};

// Section 3: digit-enable transistors on GPIOC. One bit is enabled at a time.
constexpr std::array<std::uint32_t, 4U> kDigitMasks{
    1UL << 2U,
    1UL << 3U,
    1UL << 10U,
    1UL << 12U,
};

constexpr std::uint32_t kAllDigits = kDigitMasks[0] | kDigitMasks[1]
                                   | kDigitMasks[2] | kDigitMasks[3];

}  // namespace

namespace display {

Status configureSegmentPins()
{
    // Section 4: enable GPIOA, then build one mask covering all seven two-bit
    // MODER fields and one value selecting output mode (01) for every segment.
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN_Msk;

    constexpr std::uint32_t modeMask = GPIO_MODER_MODE4_Msk
                                     | GPIO_MODER_MODE5_Msk
                                     | GPIO_MODER_MODE6_Msk
                                     | GPIO_MODER_MODE7_Msk
                                     | GPIO_MODER_MODE9_Msk
                                     | GPIO_MODER_MODE10_Msk
                                     | GPIO_MODER_MODE11_Msk;
    constexpr std::uint32_t outputModes = GPIO_MODER_MODE4_0
                                        | GPIO_MODER_MODE5_0
                                        | GPIO_MODER_MODE6_0
                                        | GPIO_MODER_MODE7_0
                                        | GPIO_MODER_MODE9_0
                                        | GPIO_MODER_MODE10_0
                                        | GPIO_MODER_MODE11_0;

    GPIOA->MODER = (GPIOA->MODER & ~modeMask) | outputModes;
    // Push-pull, low speed, no pull resistors. Clearing only owned bits avoids
    // disturbing unrelated GPIOA pins used by other modules.
    GPIOA->OTYPER &= ~kAllSegments;
    GPIOA->OSPEEDR &= ~(GPIO_OSPEEDR_OSPEED4_Msk
                      | GPIO_OSPEEDR_OSPEED5_Msk
                      | GPIO_OSPEEDR_OSPEED6_Msk
                      | GPIO_OSPEEDR_OSPEED7_Msk
                      | GPIO_OSPEEDR_OSPEED9_Msk
                      | GPIO_OSPEEDR_OSPEED10_Msk
                      | GPIO_OSPEEDR_OSPEED11_Msk);
    GPIOA->PUPDR &= ~(GPIO_PUPDR_PUPD4_Msk
                    | GPIO_PUPDR_PUPD5_Msk
                    | GPIO_PUPDR_PUPD6_Msk
                    | GPIO_PUPDR_PUPD7_Msk
                    | GPIO_PUPDR_PUPD9_Msk
                    | GPIO_PUPDR_PUPD10_Msk
                    | GPIO_PUPDR_PUPD11_Msk);

    // BSRR lower 16 bits SET outputs high. Direct assignment is correct because
    // BSRR is a write-only action register; read-modify-write has no meaning.
    GPIOA->BSRR = kAllSegments;
    return Status::kOk;
}

Status configureDigitPins()
{
    // Same pattern as segments, but for four digit-select pins on GPIOC.
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOCEN_Msk;

    constexpr std::uint32_t modeMask = GPIO_MODER_MODE2_Msk
                                     | GPIO_MODER_MODE3_Msk
                                     | GPIO_MODER_MODE10_Msk
                                     | GPIO_MODER_MODE12_Msk;
    constexpr std::uint32_t outputModes = GPIO_MODER_MODE2_0
                                        | GPIO_MODER_MODE3_0
                                        | GPIO_MODER_MODE10_0
                                        | GPIO_MODER_MODE12_0;

    GPIOC->MODER = (GPIOC->MODER & ~modeMask) | outputModes;
    GPIOC->OTYPER &= ~kAllDigits;
    GPIOC->OSPEEDR &= ~(GPIO_OSPEEDR_OSPEED2_Msk
                      | GPIO_OSPEEDR_OSPEED3_Msk
                      | GPIO_OSPEEDR_OSPEED10_Msk
                      | GPIO_OSPEEDR_OSPEED12_Msk);
    GPIOC->PUPDR &= ~(GPIO_PUPDR_PUPD2_Msk
                    | GPIO_PUPDR_PUPD3_Msk
                    | GPIO_PUPDR_PUPD10_Msk
                    | GPIO_PUPDR_PUPD12_Msk);

    // BSRR upper 16 bits RESET outputs low. `<< 16` moves each pin mask from the
    // SET half to the RESET half without touching unrelated pins.
    GPIOC->BSRR = kAllDigits << 16U;
    return Status::kOk;
}

void initialize(State& state)
{
    // Assignment from `{}` resets the complete aggregate, then calculates the
    // visible representation once so refresh never sees uninitialized digits.
    state = {};
    state.digits = encodeNumber(state.number);
}

Status calculateDigits(void* const context)
{
    // Scheduler adapter: validate, convert the generic pointer, call pure logic.
    if (context == nullptr) {
        return Status::kNullPointer;
    }
    auto& state = *static_cast<State*>(context);
    state.digits = encodeNumber(state.number);
    return Status::kOk;
}

Status refresh(void* const context)
{
    if (context == nullptr) {
        return Status::kNullPointer;
    }
    auto& state = *static_cast<State*>(context);
    // activeDigit should always be 0..3. Recover to zero if corrupted rather
    // than indexing beyond either std::array.
    const std::size_t index = (state.activeDigit < state.digits.size())
                            ? state.activeDigit : 0U;

    // Step 1: blank every digit before changing segments. This break-before-make
    // order avoids "ghost" segments from the previously selected digit.
    GPIOC->BSRR = kAllDigits << 16U;
    GPIOA->BSRR = kAllSegments;

    // Step 2: fetch through a checked index. `auto` infers uint8_t here.
    const auto digit = state.digits[index];
    // A valid value is either the blank sentinel or a decimal digit 0..9.
    // Any other corrupted value is rendered blank without indexing the table.
    if ((digit != kBlank) && (digit < kDigitPatterns.size())) {
        // Pattern holds OFF bits. Invert within kAllSegments to obtain ON bits,
        // shift to BSRR's reset half (drive low), then enable exactly one digit.
        GPIOA->BSRR = (kAllSegments & ~kDigitPatterns[digit]) << 16U;
        GPIOC->BSRR = kDigitMasks[index];
    }

    // Modulo wraps 3+1 back to 0. The explicit cast documents that 0..3 fits.
    state.activeDigit = static_cast<std::uint8_t>((index + 1U) % state.digits.size());
    return Status::kOk;
}

void enterSafeState()
{
    // Safe-state code cannot assume normal initialization completed, so it first
    // enables both GPIO clocks and then establishes known inactive output data.
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN_Msk | RCC_AHB2ENR_GPIOCEN_Msk;
    (void)RCC->AHB2ENR;

    // Establish inactive output data before selecting output mode to avoid a
    // visible or unsafe transient if this is called during early startup.
    GPIOC->BSRR = kAllDigits << 16U;
    GPIOA->BSRR = kAllSegments;

    constexpr std::uint32_t segmentModeMask = GPIO_MODER_MODE4_Msk
                                            | GPIO_MODER_MODE5_Msk
                                            | GPIO_MODER_MODE6_Msk
                                            | GPIO_MODER_MODE7_Msk
                                            | GPIO_MODER_MODE9_Msk
                                            | GPIO_MODER_MODE10_Msk
                                            | GPIO_MODER_MODE11_Msk;
    constexpr std::uint32_t segmentOutputModes = GPIO_MODER_MODE4_0
                                               | GPIO_MODER_MODE5_0
                                               | GPIO_MODER_MODE6_0
                                               | GPIO_MODER_MODE7_0
                                               | GPIO_MODER_MODE9_0
                                               | GPIO_MODER_MODE10_0
                                               | GPIO_MODER_MODE11_0;
    constexpr std::uint32_t digitModeMask = GPIO_MODER_MODE2_Msk
                                          | GPIO_MODER_MODE3_Msk
                                          | GPIO_MODER_MODE10_Msk
                                          | GPIO_MODER_MODE12_Msk;
    constexpr std::uint32_t digitOutputModes = GPIO_MODER_MODE2_0
                                             | GPIO_MODER_MODE3_0
                                             | GPIO_MODER_MODE10_0
                                             | GPIO_MODER_MODE12_0;

    GPIOA->MODER = (GPIOA->MODER & ~segmentModeMask) | segmentOutputModes;
    GPIOC->MODER = (GPIOC->MODER & ~digitModeMask) | digitOutputModes;
}

}  // namespace display
