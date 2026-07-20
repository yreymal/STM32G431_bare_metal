# Beginner guide to the C++ used in this project

This file explains the syntax that can look like “magic” when coming from C or
from direct register examples. The comments inside `Core/Inc/` and `Core/Src/` explain the
same ideas at the exact line where they are used.

## How the files connect

- `Core/Inc/name.hpp` declares the public interface: types, constants, and functions
  other modules may use.
- `Core/Src/name.cpp` includes that header and implements the functions.
- CMake supplies `-I.../Core/Inc`, so `#include "board.hpp"` finds
  `Core/Inc/board.hpp`.
- Each `.cpp` is compiled separately into an object file. The linker later
  combines those objects with startup code and libraries.

`#pragma once` prevents one header from being processed twice in the same
`.cpp`. A namespace groups names and prevents collisions:

```cpp
namespace timer6 {
Status start();
}

timer6::start();  // `::` selects start inside namespace timer6
```

## Fixed-width integers and suffixes

`std::uint32_t` is an unsigned integer with exactly 32 bits. That matters for
registers and wraparound timing. `100U` is an unsigned integer literal. The
apostrophes in `100'000'000U` are visual separators and do not change the value.

Unsigned overflow wraps modulo 2^N and is defined by C++. Signed overflow is
undefined, so hardware/timing arithmetic here deliberately uses unsigned types.

## `constexpr`, `static_assert`, and `const`

`constexpr` means the value or function can be evaluated during compilation.
Clock equations and timer divisors therefore cost no runtime calculation.

```cpp
constexpr std::uint32_t vco = (24'000'000U / 3U) * 25U;
static_assert(vco == 200'000'000U);
```

If a `static_assert` condition is false, the build stops immediately. `const`
means a particular runtime object cannot be changed through that name.

## References, pointers, and `void*`

```cpp
void initialize(State& state);       // reference: valid object, no null
Status receive(std::uint8_t& byte);  // function writes caller's byte
State* pointer = &state;              // & takes an address
pointer->number = 42U;                // -> accesses a member through a pointer
```

The scheduler must store callbacks for different modules. It therefore uses a
generic `void*` context. The registering code passes an address, and the
callback validates and converts it back:

```cpp
scheduler.addTask(display::refresh, &displayState, 4U, now);

Status refresh(void* context)
{
    if (context == nullptr) {
        return Status::kNullPointer;
    }
    auto& state = *static_cast<State*>(context);
    // use state normally
}
```

`static_cast<State*>` does not magically verify the type. Safety comes from the
registration contract: the same address was originally a `State*`.

## Function pointers and callbacks

This declaration is easiest to read through its alias:

```cpp
using Callback = Status (*)(void* context);
```

It means: “Callback is a pointer to a function receiving `void*` and returning
`Status`.” A callback lets the scheduler call a function selected during
initialization without knowing which driver owns it.

## Classes used without dynamic objects

`Uart1` contains only static functions because there is exactly one USART1
peripheral:

```cpp
class Uart1 final {
public:
    static Status initialize();
    Uart1() = delete;
};
```

`static` means no `Uart1` object is required. `= delete` makes construction a
compile error. `final` prevents inheritance. None of this allocates memory.

## RAII critical section

`CriticalSection` disables interrupts in its constructor and restores the old
state in its destructor:

```cpp
{
    const CriticalSection guard{};
    events = pendingButtons;
    pendingButtons = 0U;
}  // destructor runs here, even if code exits the scope early
```

This automatic pairing is RAII. Copy/move operations are deleted because two
guard objects must not restore the same saved interrupt state.

## `volatile` is not a lock

`volatile` tells the compiler that a value can change asynchronously and must
really be read/written. It does not make a multi-step operation atomic and does
not prevent an interrupt. The project combines it with Cortex-M aligned 32-bit
atomic accesses or a short `CriticalSection` as appropriate.

## Register bit operations

CMSIS exposes registers as fields such as `GPIOA->MODER`. Common patterns are:

```cpp
register |= mask;           // set selected bits
register &= ~mask;          // clear selected bits
register = (register & ~fieldMask) | newFieldValue;
```

`~` inverts bits, `&` keeps bits present in both operands, `|` combines bits,
and `<<` shifts a bit pattern to its register position.

Do not use read-modify-write blindly. STM32 `GPIOx->BSRR` is an action register:

```cpp
GPIOA->BSRR = pins;          // set pins high
GPIOA->BSRR = pins << 16U;   // reset pins low
```

Direct assignment changes only the selected GPIO outputs and does not disturb
other pins. EXTI pending flags are write-one-to-clear and timer flags in this
project are clear-on-zero, so their comments show the required direct writes.

## Attributes and exception policy

- `[[nodiscard]]` asks for a warning when a returned error is ignored.
- `[[noreturn]]` says a fault function never returns.
- `noexcept` says a function/destructor does not throw.
- The target CMake configuration also disables C++ exceptions and RTTI.

## `extern "C"` handlers

C++ normally changes function names to encode parameter types (“name
mangling”). The STM32 startup vector table expects exact C names such as
`SysTick_Handler`, so interrupt/exception definitions use `extern "C"`.

## Suggested reading order

1. `Core/Src/main.cpp` — see the complete startup and scheduling flow.
2. `Core/Inc/status.hpp` and `Core/Inc/scheduler.hpp` — learn statuses and callbacks.
3. `Core/Src/scheduler.cpp` plus `tests/unit_tests.cpp` — logic without hardware.
4. `Core/Inc/display_model.hpp` — small `constexpr` algorithm.
5. `Core/Src/seven_segment_display.cpp` — GPIO masks and BSRR multiplexing.
6. `Core/Src/timer6.cpp`, `Core/Src/interrupts.cpp`, `Core/Inc/critical_section.hpp` — ISR/main
   sharing.
7. `Core/Src/board.cpp` and `Core/Src/timer8.cpp` — more complex register sequences.
8. `Core/Src/safety_manager.cpp` — centralized terminal fault handling.
