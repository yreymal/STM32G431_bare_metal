# Project coding and tool policy

## Language and runtime subset

- ISO C++17, no compiler language extensions.
- Target compiler baseline must be recorded by the integrating project. The
  supplied CMake target assumes an Arm GCC-compatible compiler.
- Exceptions and RTTI are disabled for target code.
- Project code performs no dynamic allocation. Heap-providing libraries and
  containers are not used. Any future allocation requires a documented timing,
  exhaustion, ownership, and fragmentation analysis.
- Recursion is prohibited. Function pointers are limited to scheduler callbacks
  registered during initialization.
- Static object initialization must not access hardware. Current mutable state
  uses zero/constant initialization only.

## Mandatory compiler diagnostics

The project configuration enables `-Wall -Wextra -Wpedantic -Wconversion
-Wsign-conversion -Wshadow -Wundef -Wformat=2 -Wdouble-promotion -Werror`.
Target builds additionally generate `.su` stack-usage files with
`-fstack-usage`. Every warning is a build failure unless a controlled deviation
is approved outside the source.

## Safety-oriented rules

- Use fixed-width unsigned integers at registers, timers, interfaces, and stored
  state. Check narrowing and data-dependent indexing.
- Use `enum class`, `constexpr`, `std::array`, explicit initialization, internal
  linkage, and `const` wherever applicable.
- Every polling loop has a finite timeout and explicit status.
- Use direct assignment for STM32 `BSRR`, EXTI write-one-to-clear flags, and
  timer status clearing. Avoid read-modify-write on registers with special
  clearing semantics.
- ISRs must acknowledge/capture only and must never block, poll, or execute
  application behavior.
- Shared ISR/main state requires a documented atomic or critical-section
  protocol. `volatile` alone is not synchronization.
- Hardware-owner modules provide idempotent safe-state operations.
- External invalid data takes a normal status path; assertions are for
  compile-time/internal invariants only.

## MISRA and CERT status

MISRA C++:2023 is the proposed rule baseline for this educational C++ project;
the integrating organization must confirm the contractual edition, purchased
rule text, amendments, tool configuration, required checking coverage, and
deviation process. No formal MISRA compliance claim is made. CERT C++ should be
added when the project exposes security-sensitive parsing or diagnostics; the
current source contains no protocol parser or authenticated interface.

Generated CMSIS/device headers, startup assembly, linker scripts, and
third-party libraries require their own compliance classification and cannot be
covered by this project-source review. The project-owned
`Core/Drivers/Src/cmsis_device.cpp` is compiled under the normal strict C++17
warning policy.
