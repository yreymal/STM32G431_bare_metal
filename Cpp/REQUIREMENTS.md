# Software requirements

These requirements describe only the uploaded peripheral demonstrator. They do
not replace system, hardware, safety, or cybersecurity requirements.

| ID | Requirement and measurable acceptance criterion |
|---|---|
| SWR-CLK-001 | On startup, software shall switch from HSI to a 100 MHz SYSCLK derived from a 24 MHz HSE using PLLM=3, PLLN=25, PLLR=2. Every ready/switch wait shall terminate with `Status` on timeout. |
| SWR-CLK-002 | HSI shall remain enabled as fallback and HSE Clock Security System shall be enabled before Operational state. An NMI shall cause terminal Safe state. |
| SWR-CLK-003 | PA8 MCO shall output SYSCLK/16 (nominal 6.25 MHz) only after clock initialization; Safe state shall disconnect MCO. |
| SWR-DSP-001 | The display shall represent values 0–9999 with suppressed leading zeroes, refresh one of four digits every 4 ms, and never index a pattern table with an invalid value. |
| SWR-DSP-002 | Display configuration and Safe state shall disable all digit selects before changing segment drive. Unrelated GPIO bits shall not be modified through `ODR`. |
| SWR-TIM-001 | TIM6 shall generate a nominal 1 Hz update from the APB1 timer clock and increment aligned 32-bit unsigned diagnostic/seconds counters. |
| SWR-TIM-002 | TIM8 CH1/CH2 shall remain disabled through initialization and shall start only after entry to Operational state. Safe state shall clear MOE, stop the timer, and disconnect PC6/PC7. |
| SWR-SCH-001 | The cooperative scheduler shall store at most 25 statically allocated tasks, reject null callbacks and periods outside 1–`0x7FFFFFFF` ms, and perform no allocation. |
| SWR-SCH-002 | A task delayed by one or more releases shall run no more than once per scheduler pass; skipped releases shall be counted with saturation. |
| SWR-SCH-003 | A callback failure shall be counted and propagated to the caller in the same scheduler pass. |
| SWR-INT-001 | EXTI0/EXTI1 handlers shall clear their pending request, set a bounded event flag, and return without blocking or writing application output GPIO. |
| SWR-INT-002 | Button events within 50 ms of the previously accepted event shall be rejected and counted; accepted event masks shall be available in `ButtonDiagnostics`. |
| SWR-UART-001 | USART1 shall use PC4/PC5 AF7 at 9600 baud, 8N1, with finite polling waits. Disabled TX/RX use, timeout, parity, framing, noise, and overrun conditions shall return explicit status. |
| SWR-SAFE-001 | The software shall implement Startup, Operational, and terminal Safe states. Only successful completion of all initialization/registration may enter Operational. |
| SWR-SAFE-002 | Any propagated initialization/task fault or CPU fault shall disable interrupts, record status/source and SCB fault registers, command owner-defined safe outputs, and halt until external reset. |
| SWR-RT-001 | The scheduled periods shall be display refresh 4 ms, button processing 20 ms, display conversion 200 ms, and seconds transfer 400 ms. Missed cooperative releases shall be observable. |

## Assumptions and derived constraints

- The MCU is STM32G431RB with 32-bit atomic aligned loads/stores.
- Scheduler execution occurs at least once per `0x7FFFFFFF` ms and no configured
  interval exceeds that value; this makes unsigned wraparound timing valid.
- The stated GPIO safe polarities match the external display/transistor
  hardware. This must be confirmed electrically.
- HSE/PLL ranges, Flash wait-state setting, and board supply/voltage scaling must
  be confirmed against the exact MCU revision, datasheet, option bytes, and
  operating conditions during target integration.
- UART input is local bench data, not trusted vehicle/network data. No protocol,
  authentication, E2E protection, or parser is defined.
- Safe state is terminal. Automatic restart, degraded operation, and watchdog
  recovery are intentionally not claimed.
