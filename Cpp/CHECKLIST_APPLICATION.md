# Automotive checklist application report

## Metadata and interpretation

| Field | Value |
|---|---|
| Project / ECU | STM32G431RB peripheral safety demonstrator |
| Function / item | Clock, display, buttons, UART, timers, cooperative scheduler, terminal safe state |
| Platform | STM32G431RB / bare metal / C++17 |
| Baseline | Corrected source archive, 2026-07-14 |
| Safety classification | Educational QM demonstration; no ASIL claim |
| Cybersecurity classification | No external vehicle/network protocol in scope; not classified |
| Review type | Source-level checklist application, host logic test, manual MCU-code review |

`[x]` means evidence exists in this archive and was checked at the stated
scope. `[ ]` means an action remains. `N/A` is justified for this demonstrator
only; a real project requires named owners and approval for every tailoring
decision.

## 1–6. Governance, requirements, safety, diagnostics, MCU mechanisms, ASPICE

- [x] The item boundary, interfaces, environment assumptions, intended
  behavior, safe output convention, and system states are defined in
  `README.md` and `REQUIREMENTS.md`.
- [x] Software requirements have unique IDs and measurable source/verification
  acceptance criteria. Bidirectional code/test status is recorded in
  `TRACEABILITY.md`.
- [x] Startup, Operational, and terminal Safe behavior is explicit. Faults from
  initialization, scheduled callbacks, CPU exceptions, and CSS NMI use one
  controlled safe-state path.
- [x] Clock/peripheral waits are bounded and propagate status. Reset cause and
  SCB fault registers are captured for debugger diagnosis.
- [x] Clock Security System is enabled and HSI remains available as fallback.
- [x] TIM8 output enable is separated from configuration and occurs only after
  the system enters Operational. Safe state clears MOE first.
- [x] Basic software architectural, detailed-design, requirement, and
  verification evidence is present in the four project Markdown files and host
  tests.
- [ ] Applicable markets, OEM requirements, type approval, legal obligations,
  IATF 16949, organization roles, independence, supplier responsibilities, and
  approved safety/quality plans are not available in a source-only demo.
- [ ] ISO 26262 tailoring, HARA, safety goals, FSC/TSC, FMEA/FMEDA/FTA/DFA,
  diagnostic coverage, FTTI, and safety case require a defined vehicle item and
  independent review. The code changes do not create compliance.
- [ ] Automotive SPICE process ownership, baselines, problem/change records,
  review records, and SWE.1–SWE.6 assessment evidence require the integrating
  organization and repository history.
- [ ] Hardware watchdog manager/checkpoints, Flash CRC, RAM startup/periodic
  tests, stack guards/measurement, MPU policy, voltage/brownout verification,
  independent clock-frequency monitoring, and fault injection are not
  implemented or verified.
- N/A Lockstep, PBIST/LBIST, ECC/parity, and other device mechanisms not exposed
  by this source must be assessed against the exact STM32G431 safety manual,
  reference manual, silicon revision, and project ASIL needs.
- N/A Sensor/actuator plausibility and the parking-sensor “uncertain is not
  clear” rule are outside this peripheral demonstrator; no parking-sensor input
  is present.
- N/A SOTIF and ISO/PAS 8800 are not applicable to the current non-ADAS,
  non-perception, non-AI function. Reassess if parking assistance is added.

## 7–9. Coding-standard, C, and C++ review

- [x] C++17, warnings-as-errors, fixed-capacity storage, no project heap use,
  no exceptions/RTTI on target, ISR rules, and deviation expectations are
  defined in `CODING_STANDARD.md` and `CMakeLists.txt`.
- [x] Fixed-width types are used at hardware/state boundaries. Compile-time
  assertions check PLL/timer/display assumptions and array capacities.
- [x] All project variables are initialized; hardware/data-dependent indexes are
  checked; callbacks reject invalid pointers; private objects have internal
  linkage; immutable configuration is `constexpr`.
- [x] Polling loops are bounded. Failure-capable APIs return explicit `Status`,
  `[[nodiscard]]` is used, and main propagates failures to the safety manager.
- [x] STM32 `GPIOx->BSRR` uses direct assignment. EXTI W1C flags and timer
  clear-on-zero flags use direct writes rather than unsafe register RMW.
- [x] ISR work is acknowledge/capture only. Main/ISR flag transfer uses a
  bounded RAII critical section; `volatile` is not treated as mutual exclusion.
- [x] `enum class`, `constexpr`, `std::array`, deleted copy operations for the
  critical-section guard, and deterministic object lifetime are used.
- [x] The delayed scheduler path is bounded: there is no unbounded catch-up
  loop, and counters saturate rather than wrap silently.
- [x] Dead/conflicting APIs were removed: the PA5 status-LED owner, busy-loop
  delay, display test output, and timer toggle operation.
- [x] Button processing no longer writes display GPIO; hardware ownership is
  separated.
- [ ] A qualified/configured static-analysis run against the contractually
  selected MISRA C++ rule set was not possible in this environment. Formal
  rule coverage, diagnostics, deviations, and compliance summary remain open.
- [x] Arm debug and release targets compile and link without diagnostics using
  Arm GNU C++ 10.3.1. Both builds generate ELF, HEX, BIN, and map artifacts;
  the startup/CMSIS ABI symbols were checked for unmangled linkage.
- N/A The checklist's C-specific review section applies only to any C startup,
  system, or library sources later integrated; the supplied project files are
  C++.

## 10–12. Architecture, real time, communication, diagnostics

- [x] Hardware drivers, pure display conversion, scheduling, diagnostics, and
  safe-state supervision have distinct modules and narrow interfaces.
- [x] No god class, deep inheritance, uncontrolled singleton object,
  application processing in ISR, unbounded queue, hidden project heap use, or
  delay-based synchronization is present.
- [x] UART polling is bounded, validates enabled state, and reports parity,
  framing, noise, overrun, and timeout errors.
- [x] Periods are documented; missed releases, dispatches, and callback failures
  are observable. The scheduler and all arrays have fixed capacity.
- [ ] Deadlines, WCET, jitter, priority rationale, interrupt-load response time,
  stack use, CPU/RAM/Flash margin, and maximum blocking time require target
  measurement. A 4 ms period is not evidence that the deadline is met.
- [ ] Button debounce, UART loopback/error paths, MCO, TIM6, TIM8, safe-output
  transients, and simultaneous interrupt load require target/HIL tests.
- N/A AUTOSAR E2E, SecOC, CAN/LIN/DoIP/UDS validation, bounded communication
  queues, diagnostic authentication, and parser fuzzing are not applicable
  because no vehicle communication stack or packet parser exists in scope.

## 13–15. Cybersecurity, updates, privacy

- N/A Secure boot, firmware signing, anti-rollback, HSM/key lifecycle, SecOC,
  OTA recovery, and update campaign evidence are outside the uploaded
  application-source boundary. They become mandatory assessment topics for a
  production connected ECU and cannot be solved in these `.cpp` files alone.
- N/A The current function collects no VIN, location, driver behavior, phone,
  image, biometric, profile, or other personal data and sends no telemetry.
  GDPR/ePrivacy/EU Data Act processing requirements therefore do not apply to
  the defined demo behavior. Reassess immediately if logging or connectivity is
  added.
- [ ] Production debug-lock/option-byte policy, firmware authenticity, SBOM,
  vulnerability monitoring, and TARA are integrating-product responsibilities
  and are not evidenced here.

## 16–17. Verification and release readiness

- [x] Host tests cover display boundaries, invalid scheduler inputs, fixed
  capacity, normal releases, missed releases, callback failure, and 32-bit time
  wraparound.
- [x] Host logic compiles with the documented strict warnings as errors.
- [x] Requirements identify target verification methods and open gaps rather
  than treating source inspection as hardware verification.
- [ ] No statement/branch/MC/DC coverage measurement, sanitizer campaign,
  target unit test, SIL/PIL/HIL test, long-duration test, fault injection, or
  target resource/timing measurement is included.
- [ ] The project is not release-ready: formal reviews, approved MISRA results,
  trace completeness, hardware safety mechanisms, production
  configuration, reproducible final binary, anomaly acceptance, and safety /
  cybersecurity cases remain open.

## Important code corrections made

| Finding | Correction |
|---|---|
| Scheduler could repeatedly “catch up” after a delay and starve other work | Skip overdue releases, execute once per pass, and count missed releases |
| Scheduler callbacks returned `void` | Return and propagate `Status`; count failures |
| Button task wrote segment GPIO owned by display | Record debounced input diagnostics only |
| HSI fallback was disabled after PLL switch | Keep HSI enabled and activate CSS/NMI reaction |
| Timer ISR used read-modify-write to clear status | Use direct clear-on-zero assignment |
| TIM8 was enabled during configuration | Separate configure/start; start after Operational; safe state clears MOE |
| Fault cause was discarded or immediately reset | Central terminal safe state with status/source/reset/SCB debug record |
| Busy-loop delay and conflicting PA5 LED API remained | Remove both from public/source interfaces |
| UART ignored receive error flags and disabled-state use | Check/clear errors and return explicit status |
| Display conversion mixed pure logic and GPIO | Extract `constexpr` encoder and add host boundary tests |
| No deterministic build/check policy | Add strict CMake targets, coding policy, requirements, traceability, and tests |

## Action register

| ID | Action | Priority | Verification / exit criterion |
|---|---|---|---|
| A-01 | Confirm schematic pin map, digit polarity, PA5 LED loading, HSE path, voltage range, and option bytes | P0 before flashing | Signed hardware review plus measured inactive output levels |
| A-02 | Completed: integrate startup/system/linker/CMSIS files and perform clean Arm debug/release builds | Closed 2026-07-14 | Zero diagnostics under documented compiler/options; ELF/HEX/BIN/map generated |
| A-03 | Execute target smoke and fault tests for clock, UART, buttons, timers, display, CPU faults, and Safe state | P0 | Test report with measured outputs and injected failures |
| A-04 | Define safety classification and perform item definition/HARA/FMEA if used beyond bench learning | P0 for vehicle use | Approved safety work products and derived requirements |
| A-05 | Design a supervised hardware-watchdog manager | P1 | Independent checkpoints, window/startup policy, reset-cause and fault-injection tests |
| A-06 | Add Flash/RAM integrity, stack monitoring, voltage/brownout and configuration checks as required | P1 | Mechanism requirements, diagnostic coverage, target tests |
| A-07 | Measure WCET, jitter, interrupt latency, stack, CPU/RAM/Flash margins and overload behavior | P1 | Worst-case report meeting approved budgets |
| A-08 | Run controlled MISRA C++/static analysis and manage deviations | P1 | Approved compliance summary with zero unreviewed diagnostics |
| A-09 | Add CI for host tests, target compile, static analysis, artifact hashes, and retained logs | P1 | Blocking pipeline and reproducible baseline evidence |
| A-10 | Reassess TARA, secure boot/update/debug locking, E2E/SecOC, and privacy when connectivity or vehicle data is introduced | P0 when scope changes | Approved cybersecurity/privacy impact analysis |
