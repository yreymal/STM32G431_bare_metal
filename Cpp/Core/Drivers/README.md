# STM32 drivers in the C++17 target

The CMSIS headers in this directory are the unmodified ST/Arm vendor files.
Although their public syntax is C-compatible, they already contain the guards
required for inclusion from C++ and should not be renamed to `.hpp` or wrapped
in C++ classes merely to change their language appearance.

`CMakeLists.txt` defines the `STM32::CMSIS` interface target. It provides:

- the STM32G431 device selection macro;
- the CMSIS device and Cortex-M include paths as system includes;
- the C++17 language requirement; and
- `Src/cmsis_device.cpp`, which supplies `SystemInit`, `SystemCoreClock`, and
  the CMSIS clock tables/update function.

The system symbols and interrupt handlers use `extern "C"` because startup
assembly refers to their unmangled ABI names. Application-facing drivers remain
normal namespaced C++17 code under `Core/Inc` and `Core/Src`.
