/*
 * TIM6 one-second time-base driver interface.
 *
 * Initialization and starting are separate so outputs/interrupt activity begin
 * only after the rest of the system has initialized successfully.
 */
#pragma once

#include <cstdint>

#include "status.hpp"

namespace timer6 {

[[nodiscard]] Status initialize();
[[nodiscard]] Status start();
void stop();
[[nodiscard]] std::uint32_t seconds();
[[nodiscard]] std::uint32_t ticksSeen();
// Scheduler callbacks must match Status(void*), hence the generic context.
[[nodiscard]] Status storeSeconds(void* context);

}  // namespace timer6
