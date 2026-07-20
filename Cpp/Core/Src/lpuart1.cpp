/*
 * Register-level LPUART1 implementation on PA2/PA3.
 *
 * The code is polling based for clarity, but every poll has a timeout and every
 * receiver error flag is converted into an explicit Status.
 */
#include "lpuart1.hpp"

#include "clock_config.hpp"
#include "stm32g431xx.h"

namespace {

// LPUART uses a special 256 * f_clock / baud BRR formula. `ULL` forces the
// intermediate multiplication to 64 bits so 256 * 100 MHz cannot overflow.
constexpr std::uint32_t kBaudRate = 115'200U;
constexpr std::uint32_t kBaudRegister = static_cast<std::uint32_t>(
    ((256ULL * clock_config::kSystemClockHz) + (kBaudRate / 2U)) / kBaudRate);
constexpr std::uint32_t kPollingTimeout = 1'000'000U;
constexpr std::uint32_t kReceiveErrorFlags = USART_ISR_PE_Msk
                                           | USART_ISR_FE_Msk
                                           | USART_ISR_NE_Msk
                                           | USART_ISR_ORE_Msk;
constexpr std::uint32_t kReceiveErrorClearFlags = USART_ICR_PECF_Msk
                                                | USART_ICR_FECF_Msk
                                                | USART_ICR_NECF_Msk
                                                | USART_ICR_ORECF_Msk;

static_assert(kBaudRegister >= 0x300U && kBaudRegister <= 0xF'FFFFU);

Status checkAndClearReceiveError()
{
    // Same error policy as USART1: clear all receive errors and report failure.
    if ((LPUART1->ISR & kReceiveErrorFlags) != 0U) {
        LPUART1->ICR = kReceiveErrorClearFlags;
        return Status::kPeripheralFault;
    }
    return Status::kOk;
}

}  // namespace

namespace serial {

void LowPowerUart1::configurePins()
{
    // PA2/PA3 alternate function 12, push-pull, medium speed, RX pull-up.
    // The APB clock enable here is harmlessly repeated by initialize().
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN_Msk;
    RCC->APB1ENR2 |= RCC_APB1ENR2_LPUART1EN_Msk;

    GPIOA->MODER = (GPIOA->MODER
                    & ~(GPIO_MODER_MODE2_Msk | GPIO_MODER_MODE3_Msk))
                   | (2UL << GPIO_MODER_MODE2_Pos)
                   | (2UL << GPIO_MODER_MODE3_Pos);
    GPIOA->AFR[0] = (GPIOA->AFR[0]
                     & ~(GPIO_AFRL_AFSEL2_Msk | GPIO_AFRL_AFSEL3_Msk))
                    | (12UL << GPIO_AFRL_AFSEL2_Pos)
                    | (12UL << GPIO_AFRL_AFSEL3_Pos);
    GPIOA->OTYPER &= ~(GPIO_OTYPER_OT2_Msk | GPIO_OTYPER_OT3_Msk);
    GPIOA->OSPEEDR = (GPIOA->OSPEEDR
                      & ~(GPIO_OSPEEDR_OSPEED2_Msk | GPIO_OSPEEDR_OSPEED3_Msk))
                     | (2UL << GPIO_OSPEEDR_OSPEED2_Pos)
                     | (2UL << GPIO_OSPEEDR_OSPEED3_Pos);
    GPIOA->PUPDR = (GPIOA->PUPDR
                    & ~(GPIO_PUPDR_PUPD2_Msk | GPIO_PUPDR_PUPD3_Msk))
                   | (1UL << GPIO_PUPDR_PUPD3_Pos);
}

Status LowPowerUart1::initialize()
{
    // Disable UE before changing format/clock fields. LPUART1SEL=01 selects
    // SYSCLK, matching the frequency used in kBaudRegister above.
    RCC->APB1ENR2 |= RCC_APB1ENR2_LPUART1EN_Msk;
    LPUART1->CR1 &= ~USART_CR1_UE_Msk;
    LPUART1->CR1 &= ~(USART_CR1_M0_Msk | USART_CR1_M1_Msk);
    RCC->CCIPR = (RCC->CCIPR & ~RCC_CCIPR_LPUART1SEL_Msk)
               | (1UL << RCC_CCIPR_LPUART1SEL_Pos);
    LPUART1->BRR = kBaudRegister;
    LPUART1->CR2 &= ~USART_CR2_STOP_Msk;
    LPUART1->CR3 &= ~(USART_CR3_DMAT_Msk | USART_CR3_DMAR_Msk);
    LPUART1->ICR = kReceiveErrorClearFlags;
    LPUART1->CR1 |= USART_CR1_UE_Msk | USART_CR1_TE_Msk | USART_CR1_RE_Msk;
    // TEACK and REACK are hardware acknowledgements, not just our requested bits.
    std::uint32_t timeout = kPollingTimeout;
    while ((LPUART1->ISR & (USART_ISR_TEACK_Msk | USART_ISR_REACK_Msk))
           != (USART_ISR_TEACK_Msk | USART_ISR_REACK_Msk)) {
        if (--timeout == 0U) {
            return Status::kPeripheralTimeout;
        }
    }
    return (LPUART1->BRR == kBaudRegister)
         ? Status::kOk
         : Status::kPeripheralFault;
}

Status LowPowerUart1::send(const std::uint8_t byte)
{
    // Require peripheral + transmitter enable before polling TXE.
    constexpr std::uint32_t requiredEnableBits = USART_CR1_UE_Msk
                                               | USART_CR1_TE_Msk;
    if ((LPUART1->CR1 & requiredEnableBits) != requiredEnableBits) {
        return Status::kInvalidState;
    }
    std::uint32_t timeout = kPollingTimeout;
    while ((LPUART1->ISR & USART_ISR_TXE_Msk) == 0U) {
        if (--timeout == 0U) {
            return Status::kPeripheralTimeout;
        }
    }
    LPUART1->TDR = static_cast<std::uint32_t>(byte);
    return Status::kOk;
}

Status LowPowerUart1::receive(std::uint8_t& byte)
{
    // Require receiver enable and prioritize explicit UART errors over timeout.
    constexpr std::uint32_t requiredEnableBits = USART_CR1_UE_Msk
                                               | USART_CR1_RE_Msk;
    if ((LPUART1->CR1 & requiredEnableBits) != requiredEnableBits) {
        return Status::kInvalidState;
    }
    std::uint32_t timeout = kPollingTimeout;
    while ((LPUART1->ISR & USART_ISR_RXNE_RXFNE_Msk) == 0U) {
        const auto errorStatus = checkAndClearReceiveError();
        if (errorStatus != Status::kOk) {
            return errorStatus;
        }
        if (--timeout == 0U) {
            return Status::kPeripheralTimeout;
        }
    }
    const auto errorStatus = checkAndClearReceiveError();
    if (errorStatus != Status::kOk) {
        return errorStatus;
    }
    byte = static_cast<std::uint8_t>(LPUART1->RDR & 0xFFU);
    return Status::kOk;
}

}  // namespace serial
