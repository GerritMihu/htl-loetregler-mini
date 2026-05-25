# Copilot Instructions

## Build, test, and lint

- The active Pico firmware workflow now uses **PlatformIO** from the repository root.
- Main commands:
  - `pio run -e pico` builds the firmware.
  - `pio run -e pico -t upload` uploads to the Raspberry Pi Pico.
  - `pio test -e native` runs the host-side test suite.
  - `pio test -e native -f test_logic` runs a single test target.
  - `pio check -e pico` runs static analysis.
- In this development environment PlatformIO is installed in a local virtual environment, so the working command is `.venv-pio/bin/pio ...`. Do not bake that path into committed documentation unless the repository intentionally standardizes on it.
  - `platformio.ini` uses the Raspberry Pi Pico with the **Pico SDK** via the `maxgerhardt/platform-raspberrypi` PlatformIO platform.
- The older Arduino sketches under `arduino/` are still useful as reference material, but they are no longer the primary path for Pico firmware work.

## High-level architecture

- This is a mixed hardware/firmware repository for the **HTL Lötregler Mini** soldering-station controller:
  - `kicad/` contains the PCB and schematic sources.
  - `3d/` contains enclosure/mechanical assets.
  - `arduino/` contains the older firmware variants and experiments.
  - `src/`, `include/`, `lib/`, and `test/` contain the active PlatformIO-based Pico firmware.
- The current firmware is split into small, explainable pieces:
  - `src/main.c` owns the device runtime, OLED screens, heater control, button handling, and safety shutdown flow.
  - `src/button_input.c` / `include/button_input.h` provide debounced button events.
  - `src/settings_store.c` / `include/settings_store.h` load and save menu settings in flash.
  - `src/ssd1306_simple.c` / `include/ssd1306_simple.h` provide a small custom OLED driver in C.
  - `lib/LoetreglerLogic/` contains hardware-independent logic for defaults, limits, menu navigation, and editable values.
  - `test/test_logic/` exercises that pure logic on the host with PlatformIO's native test environment.
- The runtime loop is still intentionally simple and synchronous: read buttons, update state, read sensors, drive the heater with a simple control rule, apply safety checks, then redraw the current screen.
- Legacy or reference firmware still exists:
  - `arduino/htl-loetregler-firmware/` and `arduino/platformio/` contain earlier sketch-based versions.
  - `arduino/pico-loetregler/` and `arduino/RP2040/ssd1306_i2c/` are experimental Pico SDK paths.

## Key conventions

- **Keep the Pico firmware simple and teachable.** Prefer short functions, direct control flow, and explicit state over clever abstractions or heavy framework patterns.
- **Prefer C for the active Pico firmware.** The current root-level firmware is written in C with Pico SDK rather than C++/Arduino abstractions.
- **Keep the existing German domain vocabulary.** Core identifiers and UI strings use names like `tempSoll`, `temperaturSpitze`, `spannungBatterie`, `selbsthaltung`, and `abschalten`; new firmware code should follow the same terminology instead of renaming concepts into English.
- **Put testable logic in `lib/LoetreglerLogic/`.** Menu behavior, limits, defaults, and value transformations should stay hardware-independent so they can be covered by `pio test -e native`.
- **Keep hardware access in `src/`.** ADC reads, OLED drawing, button scanning, PWM output, and flash persistence belong in the firmware sources, not in the pure logic library.
- **Settings are runtime-configurable, not compile-time macros.** Startup temperature, greeting, max temperature, shutdown time, standby temperature, and step size are now menu settings stored in flash.
- **Button handling is event-driven.** Consume `Pressed` events from the debounced button layer instead of reading raw GPIO state directly inside feature logic.
- **Safety logic stays central and visible.** Low-voltage shutdown, tip-sensor failure handling, cooling before power-off, and automatic timeout are core behavior and should stay easy to trace from `main.cpp`.
- **Treat `arduino/libraries/` and external PlatformIO libs as third-party code.** Prefer adjusting project code and configuration first; only patch dependencies when the task explicitly requires it.
