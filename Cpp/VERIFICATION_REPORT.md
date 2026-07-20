# Verification report

Date: 2026-07-14

Current integration toolchains: AppleClang 17.0.0 (host) and Arm GNU C++
10.3.1 (STM32). The separately recorded GCC analyzer check used GCC 13.3.0.

Scope: hardware-independent scheduler/display logic and STM32 build integration

## Executed checks

| Check | Configuration | Result |
|---|---|---|
| Strict host compile and unit tests | C++17; `-Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Wshadow -Wundef -Wformat=2 -Wdouble-promotion -Werror` | Pass — `All host unit tests passed` |
| CMake host preset | `cmake --preset host-debug`; build; CTest | Pass — 1/1 test |
| STM32 debug firmware | Arm GNU C++ 10.3.1; C++17; Cortex-M4 hard-float; warnings as errors; exceptions/RTTI disabled | Pass — ELF/HEX/BIN/map generated; Flash 7,072 B, RAM 1,624 B |
| STM32 release firmware | Same target policy with release optimization | Pass — ELF/HEX/BIN/map generated; Flash 3,824 B, RAM 1,624 B |
| C ABI symbol check | Global ELF symbols used by startup/vector table | Pass — unmangled `SystemInit`, `SystemCoreClock`, `main`, NMI, SysTick, and TIM6 handler symbols |
| Undefined/address sanitizer run | Same warnings plus `-fsanitize=address,undefined -fno-omit-frame-pointer`; leak detection disabled because the execution sandbox uses tracing | Pass — `All host unit tests passed`; no ASan/UBSan finding |
| GCC path-sensitive analyzer | Same warnings plus `-fanalyzer` on `Core/Src/scheduler.cpp` and the test translation unit | Pass — no diagnostic |
| Folder/build-path check | No root-level `.cpp`/`.hpp`; 15 headers under `Core/Inc/`, 12 implementations under `Core/Src/`; CMakePresets parsed as JSON | Pass |

## Covered behavior

- Display encoding for 0, 42, 1000, 9999, and saturation of 65535.
- Scheduler rejection of null callback, zero period, and a period greater than
  `0x7FFFFFFF` ms.
- Normal release, delayed release without catch-up burst, and missed-release
  counting.
- Fixed-capacity overflow rejection.
- Callback-failure propagation and diagnostic counting.
- Unsigned millisecond wraparound scheduling.

## Checks not executed

The source now has a successful target compile/link and binary-generation
result. That result proves toolchain integration, C++17 compilation, ABI symbol
resolution, and linker placement; it does not prove behavior on the MCU.

No target/HIL test, electrical safe-level check, timer/MCO measurement,
interrupt-load test, WCET/stack measurement, formal MISRA tool run, coverage
measurement, or MCU fault-injection campaign was possible here. These remain in
the action register in `CHECKLIST_APPLICATION.md`.
