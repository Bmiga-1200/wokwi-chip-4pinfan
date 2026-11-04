# Project Context: 4-Pin Fan Wokwi Custom Chip

## Architecture Overview
This standalone repository implements a simulated 4-pin PC cooling fan for the Wokwi simulation platform using WebAssembly (`wasm32-unknown-wasi`).

## Tech Stack
- **Language**: C99 (compiled with Clang/LLVM WASI toolchain)
- **Target Platform**: Wokwi Simulator Custom Chip API (`wokwi-api.h`)
- **Simulation Interface**:
  - `PWM`: Input pull-up pin watched on both edges to compute high/low timing and duty cycle.
  - `TACH`: Output pin toggled via high-precision timer callbacks (2 pulses per revolution).
  - `POWER`, `GND`: Power reference pins.
  - `max_rpm`: Configurable numeric attribute slider (500-8000 RPM).
- **Test Harness**: `arduino-cli` with `AVR_PWM` library running against `arduino:avr:uno`.
- **CI/CD**: GitHub Actions workflow compiling via `wokwi/wokwi-chip-clang-action` and releasing `chip.zip` on semver tags.
