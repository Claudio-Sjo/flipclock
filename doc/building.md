# Building

## Prerequisites

- [Pico SDK](https://github.com/raspberrypi/pico-sdk) — set `PICO_SDK_PATH` environment variable
- [Pimoroni Pico libraries](https://github.com/pimoroni/pimoroni-pico) — cloned alongside the project directory
- CMake ≥ 3.13
- ARM GCC toolchain (`arm-none-eabi-gcc`)

## Directory Layout

The build expects `pimoroni-pico` to be located alongside the project:

```
parent/
  ├── flipclock/           ← this project
  └── pimoroni-pico/       ← Pimoroni libraries
```

## Build Steps

```bash
mkdir build
cd build
cmake ..
make -j$(nproc)
```

This produces `flipclock.uf2` in the build directory.

## Flashing

1. Hold the BOOTSEL button on the Pico and connect it via USB
2. Copy `build/flipclock.uf2` to the mounted RPI-RP2 drive
3. The Pico will reboot and start the clock

## Font Generator

The `fontgenerator/` directory contains a standalone tool that converts bitmap font data into the span-based `digitFont[]` format used by the firmware. It is not part of the firmware build.

```bash
cd fontgenerator
gcc fontgen.c -o fontgen
./fontgen > output.h
```
