# Requirement traceability and verification matrix

| Requirement | Design / source evidence | Verification evidence | Status |
|---|---|---|---|
| SWR-CLK-001 | `Core/Inc/clock_config.hpp`, `Core/Src/board.cpp::configureClock` | Compile-time PLL assertions; target clock measurement still required | Partial |
| SWR-CLK-002 | `Core/Src/board.cpp`, `Core/Src/interrupt_handlers.cpp`, `Core/Src/safety_manager.cpp` | Code review; HSE-failure injection still required | Partial |
| SWR-CLK-003 | `Core/Src/board.cpp::configureClockOutput`, `disableClockOutput` | Compile-time 6.25 MHz assertion; target MCO measurement required | Partial |
| SWR-DSP-001 | `Core/Inc/display_model.hpp`, `Core/Src/seven_segment_display.cpp` | `tests/unit_tests.cpp`: zero, normal, interior zero, upper/out-of-range boundaries | Verified on host logic |
| SWR-DSP-002 | `Core/Src/seven_segment_display.cpp` BSRR/safe-state ordering | Code review; oscilloscope transient test required | Partial |
| SWR-TIM-001 | `Core/Src/timer6.cpp`, `Core/Inc/clock_config.hpp` | Compile-time divider checks; target period measurement required | Partial |
| SWR-TIM-002 | `Core/Src/timer8.cpp`, `Core/Src/main.cpp`, `Core/Src/safety_manager.cpp` | Code review; fault-injection/output measurement required | Partial |
| SWR-SCH-001 | `Core/Inc/scheduler.hpp`, `Core/Src/scheduler.cpp` | Host tests for null, zero/large periods, capacity | Verified on host |
| SWR-SCH-002 | `Core/Src/scheduler.cpp` | Host tests for late release and 32-bit time wrap | Verified on host |
| SWR-SCH-003 | `Core/Src/scheduler.cpp` | Host callback-failure propagation/count test | Verified on host |
| SWR-INT-001 | `Core/Src/interrupts.cpp` handlers and scheduled processing | Code review; target maximum interrupt-load test required | Partial |
| SWR-INT-002 | `Core/Src/interrupts.cpp`, `ButtonDiagnostics` | Target bounce/event-injection test required | Open target test |
| SWR-UART-001 | `Core/Src/uart1.cpp` | Code review; loopback, error, and timeout target tests required | Partial |
| SWR-SAFE-001 | `Core/Inc/safety_manager.hpp`, `Core/Src/safety_manager.cpp`, `Core/Src/main.cpp` | State-transition target tests required | Partial |
| SWR-SAFE-002 | `Core/Src/safety_manager.cpp`, `Core/Src/interrupt_handlers.cpp` | CPU/peripheral fault-injection campaign required | Open target test |
| SWR-RT-001 | `Core/Src/main.cpp`, `Core/Src/scheduler.cpp` | Period values reviewed; WCET/jitter/overload measurement required | Partial |

“Verified on host logic” applies only to the hardware-independent behavior and
does not imply MCU, compiler, electrical, integration, or ISO 26262 verification.
