# Contributing to Wokwi 4-Pin Fan Chip

Thank you for your interest in improving the 4-Pin Fan custom chip simulation model! This guide explains the internal architecture, mathematical modeling, development environment setup, and contribution standards.

---

## Architecture & Internal Mechanics

The chip is implemented in C using the [Wokwi Custom Chips API](https://docs.wokwi.com/chips-api/getting-started) and compiled to WebAssembly (`wasm32-unknown-wasi`).

### State Structure (`chip_state_t`)
```c
typedef struct {
  pin_t pin_fan_tach;
  pin_t pin_fan_pwm;
  uint8_t pin_state_tach;
  uint32_t max_rpm_attr;
  timer_t timer_id_tach;
  uint32_t riseTime;
  uint32_t fallTime;
  uint32_t highTime;
  uint32_t lowTime;
  uint32_t period;
} chip_state_t;
```

### 1. PWM Detection via Edge Monitoring (`pin_state_change_callback`)
* The `PWM` pin is configured with `INPUT_PULLUP` and monitored using `pin_watch` for `BOTH` (rising and falling) edges.
* Timestamps are retrieved with `get_sim_nanos() >> 11` (scaled to microseconds).
* On a rising edge (`HIGH`), `period` and `lowTime` are calculated, and `riseTime` is updated.
* On a falling edge (`LOW`), `highTime` and `fallTime` are calculated, and `fallTime` is updated.
* The effective duty cycle is determined as:
  $$\text{Duty Cycle} = \frac{\text{highTime}}{\text{period}}$$

### 2. Tachometer Frequency Synthesis (`chip_timer_tach_callback`)
* The timer callback calculates effective RPM based on duty cycle and the user-configured `max_rpm`:
  $$\text{RPM} = \text{max\_rpm} \times \frac{\text{highTime}}{\text{period}}$$
* Standard PC fans produce 2 pulses per revolution. Each pulse consists of one HIGH and one LOW transition (4 state changes per revolution):
  $$\text{Frequency} = \left(\frac{\text{RPM}}{60}\right) \times 4$$
* The timer interval in microseconds is set to:
  $$\text{Interval} = \frac{1\,000\,000}{\text{Frequency}}$$
* The timer toggles `pin_fan_tach` on each expiration, generating a synthetic square-wave pulse train.

---

## Local Development Setup

### Option 1: Using WASI-SDK Directly
1. Download and install [wasi-sdk](https://github.com/WebAssembly/wasi-sdk/releases) (version 20 or newer) to `/opt/wasi-sdk`.
2. Compile the WASM binary:
   ```bash
   make
   ```
3. The compiled binary will be placed at `dist/4pinfan.chip.wasm`.

### Option 2: Using Wokwi CLI
```bash
wokwi-cli chip compile src/4pinfan.chip.c
```

### Option 3: Using Docker
```bash
docker run --rm -v $(pwd):/src -w /src ghcr.io/webassembly/wasi-sdk \
  clang --target=wasm32-unknown-wasi --sysroot=/opt/wasi-sdk/share/wasi-sysroot \
  -nostartfiles -Wl,--import-memory -Wl,--export-table -Wl,--no-entry -Werror \
  -o dist/4pinfan.chip.wasm src/4pinfan.chip.c
```

---

## Running Automated Tests

Firmware verification tests are located under `tests/`. Run them via:
```bash
make test
```
This requires `arduino-cli`. The test installs:
* `arduino:avr` platform core
* `AVR_PWM` library
* Compiles `tests/uno-pwm-tach/uno-pwm-tach.ino` targeting Arduino Uno.

---

## Pull Request Guidelines

1. **Keep Pull Requests Focused**: Limit changes to a single feature or bug fix.
2. **Follow Existing Code Style**: Format C code with `clang-format -style=Google`.
3. **Verify Tests Pass**: Always ensure `make test` completes cleanly before pushing.
4. **Update Documentation**: If modifying pin interfaces or attributes, update `4pinfan.chip.json`, `chip.json`, and `README.md`.

---

## Release Process

Releases are fully automated via GitHub Actions:
1. Ensure your changes are merged into `main`.
2. Create and push a semver git tag starting with `v`:
   ```bash
   git tag -a v1.0.0 -m "Release v1.0.0"
   git push origin v1.0.0
   ```
3. The `.github/workflows/build.yaml` workflow triggers, compiles `4pinfan.chip.wasm`, runs `make test`, packages `chip.zip`, and attaches it to the GitHub Release.
