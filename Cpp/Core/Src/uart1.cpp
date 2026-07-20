/*
 * Register-level USART1 implementation on PC4/PC5.
 *
 * This is a deterministic byte driver, not a complete communication protocol.
 * Framing, freshness, CRC/E2E, buffering, and application parsing would belong
 * in higher layers.
 */
#include "uart1.hpp"

#include "clock_config.hpp"
#include "stm32g431xx.h"

namespace {

// Section 1: compile-time serial timing. Adding baud/2 before integer division
// rounds to the nearest BRR value instead of always rounding down.
constexpr std::uint32_t kBaudRate = 9'600U;
constexpr std::uint32_t kBaudRegister =
    (clock_config::kSystemClockHz + (kBaudRate / 2U)) / kBaudRate;
constexpr std::uint32_t kPollingTimeout = 1'000'000U;
// Group all receive errors into masks: ISR bits report them; matching ICR bits
// clear them. OR joins independent one-bit flags into one test/write.
constexpr std::uint32_t kReceiveErrorFlags = USART_ISR_PE_Msk
                                           | USART_ISR_FE_Msk
                                           | USART_ISR_NE_Msk
                                           | USART_ISR_ORE_Msk;
constexpr std::uint32_t kReceiveErrorClearFlags = USART_ICR_PECF_Msk
                                                | USART_ICR_FECF_Msk
                                                | USART_ICR_NECF_Msk
                                                | USART_ICR_ORECF_Msk;

static_assert(kBaudRegister > 0U && kBaudRegister <= 0xFFFFU);

Status checkAndClearReceiveError()
{
    // ICR is write-one-to-clear, so use direct assignment—not `|=`.
    if ((USART1->ISR & kReceiveErrorFlags) != 0U) {
        USART1->ICR = kReceiveErrorClearFlags;
        return Status::kPeripheralFault;
    }
    return Status::kOk;
}

}  // namespace

namespace serial {

void Uart1::configurePins()
{
    // Section 2: PC4/PC5 use alternate-function mode, push-pull, medium speed,
    // RX pull-up, and AF7. TX needs no pull; RX should not float when disconnected.
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOCEN_Msk;

    GPIOC->MODER = (GPIOC->MODER
                    & ~(GPIO_MODER_MODE4_Msk | GPIO_MODER_MODE5_Msk))
                   | (2UL << GPIO_MODER_MODE4_Pos)
                   | (2UL << GPIO_MODER_MODE5_Pos);
    GPIOC->OTYPER &= ~(GPIO_OTYPER_OT4_Msk | GPIO_OTYPER_OT5_Msk);
    GPIOC->OSPEEDR = (GPIOC->OSPEEDR
                      & ~(GPIO_OSPEEDR_OSPEED4_Msk | GPIO_OSPEEDR_OSPEED5_Msk))
                     | (2UL << GPIO_OSPEEDR_OSPEED4_Pos)
                     | (2UL << GPIO_OSPEEDR_OSPEED5_Pos);
    GPIOC->PUPDR = (GPIOC->PUPDR
                    & ~(GPIO_PUPDR_PUPD4_Msk | GPIO_PUPDR_PUPD5_Msk))
                   | (1UL << GPIO_PUPDR_PUPD5_Pos);
    GPIOC->AFR[0] = (GPIOC->AFR[0]
                     & ~(GPIO_AFRL_AFSEL4_Msk | GPIO_AFRL_AFSEL5_Msk))
                    | (7UL << GPIO_AFRL_AFSEL4_Pos)
                    | (7UL << GPIO_AFRL_AFSEL5_Pos);
}

Status Uart1::initialize()
{
    // Section 3: enable USART1 and select SYSCLK as its kernel clock.
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN_Msk;
    RCC->CCIPR = (RCC->CCIPR & ~RCC_CCIPR_USART1SEL_Msk)
               | (1UL << RCC_CCIPR_USART1SEL_Pos);

    // Configuration fields may only be safely changed with USART disabled.
    // M0/M1=00 -> 8 data bits; PRESC=0 -> divide by one; STOP=00 -> one stop bit.
    USART1->CR1 &= ~USART_CR1_UE_Msk;
    USART1->CR1 &= ~(USART_CR1_M0_Msk | USART_CR1_M1_Msk);
    USART1->PRESC &= ~USART_PRESC_PRESCALER_Msk;
    USART1->BRR = kBaudRegister;
    USART1->CR2 &= ~USART_CR2_STOP_Msk;
    // This driver polls, so DMA is explicitly disabled. Clear old error flags
    // before enabling UART, transmitter, and receiver together.
    USART1->CR3 &= ~(USART_CR3_DMAT_Msk | USART_CR3_DMAR_Msk);
    USART1->ICR = kReceiveErrorClearFlags;
    USART1->CR1 |= USART_CR1_UE_Msk | USART_CR1_TE_Msk | USART_CR1_RE_Msk;
    // TEACK/REACK confirm that the peripheral has actually enabled both paths.
    std::uint32_t timeout = kPollingTimeout;
    while ((USART1->ISR & (USART_ISR_TEACK_Msk | USART_ISR_REACK_Msk))
           != (USART_ISR_TEACK_Msk | USART_ISR_REACK_Msk)) {
        if (--timeout == 0U) {
            return Status::kPeripheralTimeout;
        }
    }
    return (USART1->BRR == kBaudRegister)
         ? Status::kOk
         : Status::kPeripheralFault;
}

Status Uart1::send(const std::uint8_t byte)
{
    // Section 4: reject use before initialization. Both UE and TE must be set.
    constexpr std::uint32_t requiredEnableBits = USART_CR1_UE_Msk
                                               | USART_CR1_TE_Msk;
    if ((USART1->CR1 & requiredEnableBits) != requiredEnableBits) {
        return Status::kInvalidState;
    }
    std::uint32_t timeout = kPollingTimeout;
    // TXE means TDR can accept a new byte. This wait is bounded by a local
    // down-counter; timeout is returned rather than hiding a stuck peripheral.
    while ((USART1->ISR & USART_ISR_TXE_Msk) == 0U) {
        if (--timeout == 0U) {
            return Status::kPeripheralTimeout;
        }
    }
    USART1->TDR = static_cast<std::uint32_t>(byte);
    return Status::kOk;
}

Status Uart1::receive(std::uint8_t& byte)
{
    // Section 5: receiver equivalent of the initialization-state check.
    constexpr std::uint32_t requiredEnableBits = USART_CR1_UE_Msk
                                               | USART_CR1_RE_Msk;
    if ((USART1->CR1 & requiredEnableBits) != requiredEnableBits) {
        return Status::kInvalidState;
    }
    std::uint32_t timeout = kPollingTimeout;
    // Check error flags during the wait so an overrun/framing fault does not get
    // misreported later as a generic timeout.
    while ((USART1->ISR & USART_ISR_RXNE_RXFNE_Msk) == 0U) {
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
    // RDR is a 32-bit register, but 8N1 supplies one byte. Mask then narrow
    // explicitly so conversion warnings cannot hide accidental upper bits.
    byte = static_cast<std::uint8_t>(USART1->RDR & 0xFFU);
    return Status::kOk;
}

}  // namespace serial
