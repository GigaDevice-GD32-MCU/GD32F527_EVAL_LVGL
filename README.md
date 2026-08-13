# LVGL V8.3 ported to the GD32F527I EVAL

本工程将 `LVGL V8.3.11` 移植至 `GD32F527I EVAL` 平台，用于 GUI 演示。

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

> 引入新的第三方库时，请同步更新本表，并保留其许可证文本与版权声明。
