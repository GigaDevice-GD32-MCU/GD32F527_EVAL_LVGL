# LVGL V8.3 ported to the GD32F527I EVAL

This project ports LVGL V8.3.11 to the GD32F527I EVAL platform for GUI demonstration.

## Hardware Information

The `GD32F527I Development Kit` is based on:

- `GD32F527IS` microcontroller (`ARM Cortex-M33` core, FPU, DSP instructions, TrustZone security)
- `7680 KB` on-chip Flash memory and `512 KB` on-chip SRAM
- `480 x 272` RGB TFT display with capacitive touch panel
- High-speed peripheral interfaces including USB OTG, Ethernet, CAN FD, and SDIO
- On-board GD-LINK debugger/programmer for one-click download and debugging
- On-board high-precision RTC clock and temperature sensor
- Multiple user LEDs, function keys, and universal expansion pin headers

## Project Information

- GUI framework: `LVGL V8.3.11`
- Toolchain: `Keil MDK-ARM (ARM Compiler 6) / IAR / GD32EmbeddedBuilder`
- Target board: `GD32F527I EVAL`
- Display configuration: `480 x 272 / RGB565 16-bit color / landscape`

## Third-Party Components

| Category   | In use | Component | Version    | License         |
| ---------- | ------ | --------- | ---------- | --------------- |
| GUI        | `Yes`  | `LVGL`    | `V 8.3.11` | `MIT`           |
| Filesystem | `Yes`  | `FatFs`   | `R0.16`    | `FatFs license` |

> When adding new third-party libraries, update this table simultaneously and preserve the corresponding license texts and copyright statements.
