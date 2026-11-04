# chipguy_Waveshare-ESP32-S3-Touch-LCD-7-7B_board

The **board** library for two 7-inch Waveshare touch screen boards: the
**ESP32-S3-Touch-LCD-7** (800×480) and the **ESP32-S3-Touch-LCD-7B** (1024×600).
It looks after the parts of the board that every other program needs to share,
and it works out which of the two boards it is running on.

Most people never call it directly. The display library uses it for you. Install
it, and come back here when you want the battery level, the screen brightness, or
your own sensor on the I²C connector.

| Library | What it does |
| --- | --- |
| [`chipguy_Waveshare-ESP32-S3-Touch-LCD-7-7B_display`](../chipguy_Waveshare-ESP32-S3-Touch-LCD-7-7B_display/README.md) | the screen, touch and LVGL — **start here** |
| `chipguy_Waveshare-ESP32-S3-Touch-LCD-7-7B_board` (this one) | the shared I²C bus, the helper chip, which board it is, battery, brightness |

---

## Installation

Download this library from GitHub (**Code ▸ Download ZIP**), unzip it, and move the
folder into your Arduino **libraries** folder (`Documents/Arduino/libraries`).
Restart the Arduino IDE. It needs nothing else.

---

## What it gives you

```cpp
#include <ESP32S3_Touch_LCD_7_7B_Board.h>

ESP32S3_Touch_LCD_7_7B_Board board;   // the display examples already make this for you

void setup() {
    board.begin();
    Serial0.println(board.modelName());   // "ESP32-S3-Touch-LCD-7" or "...-7B"
}
```

Each board has a small **helper chip** (an "I/O expander") that switches things
the ESP32 has no spare pins for: the screen's reset, the backlight, the
memory-card slot's select line, and the touch chip's reset. The two boards use
different helper chips, and `board.begin()` tells them apart by asking a
question only one of them answers, before it changes anything. You reach all of
it through `board`:

| Call | What it does | -7 | -7B |
| --- | --- | --- | --- |
| `board.model()` / `board.modelName()` | which board this is | ✓ | ✓ |
| `board.lcdBacklight(on)` | backlight fully on or off | ✓ | ✓ |
| `board.setBacklightBrightness(percent)` | screen brightness, 0–100 | — | ✓ |
| `board.batteryVoltage()` | the battery's voltage, in volts | — | ✓ |
| `board.selectCan(true)` | use CAN instead of the second USB port | ✓ | ✓ |
| `board.Wire` | the shared I²C bus — use it for your own sensors | ✓ | ✓ |

On the -7, the calls it can't do return `false` (or "not a number" for the
battery) and change nothing, so the same sketch runs on both.

### The shared I²C bus

The touch chip and the helper chip both live on the board's **I²C bus** (two wires
that many small chips share). If your program also talks to it from more than one
place at once, the messages could get mixed up. `board.Wire` prevents that: it
works like Arduino's `Wire`, but only lets one conversation happen at a time. Use
it instead of `Wire` for anything you plug into the I²C connector.

```cpp
if (board.Wire.probe(0x44)) {        // is a sensor at address 0x44?
    board.Wire.writeRegister(0x44, 0x24, 0x00);
}
```

Each single read or write is protected automatically. To make several in a row
happen without anything in between, hold a `Guard`:

```cpp
{
    ThreadsafeWire::Guard g(board.Wire);
    uint8_t v;
    board.Wire.readRegister(0x44, 0x10, v);
    board.Wire.writeRegister(0x44, 0x10, v | 1);
}
```

Some I²C addresses are taken. The touch chip is at **0x5D** on both boards. On
the -7B the helper chip is at **0x24**. On the -7 it answers **0x20–0x27 and
0x30–0x3F**, sixteen addresses at once, so a sensor of yours can't use any of
those there. `begin()` looks at 0x38 to tell the boards apart.

### Battery (-7B)

The -7B has a socket for a single lithium battery, which charges when the USB-C
port marked **UART** is plugged in. `board.batteryVoltage()` tells you its voltage.
With no battery plugged in it still shows about 4.1–4.8 V, so a reading isn't proof
that a battery is there. Nothing on the board can switch itself off; use a battery
that has its own protection circuit. Details: [README-power.md](README-power.md).

### What else is on the board

- **RS-485 and CAN** connectors for industrial and vehicle networks.
- **I²C**, **UART** and **GPIO** connectors for your own parts.
- **Two USB-C ports:** upload through the one marked **UART**. On the -7B, set
  the small **UART1/UART2** switch beside it to **UART1**.

All of that is explained, for the -7B, in [README-board.md](README-board.md).

### Memory card

There's a microSD slot. The library doesn't include a card driver. It doesn't
share anything with the screen. See [README-sdcard.md](README-sdcard.md).

### Two boards, one set of files

The two boards are one design with a few parts changed. Each topic file below
describes the -7B and ends with a section on what differs on the -7.

> **-7 owners:** the RS-485 chip puts 5 V on one of the ESP32's pins (GPIO15),
> which it isn't built for — measured on a real board. The library switches it
> off at `board.begin()` unless your sketch uses RS-485. Don't wire anything else
> to GPIO15, and don't move the I²C header's supply resistor (H2) from 3V3 to 5V. See
> [errata 21 and 22](README-limitations-errata.md).

---

## Where to go next

- [README-board.md](README-board.md) — developers: the connectors, RS-485, CAN,
  USB, buttons, and which pins are free (both boards).
- [README-power.md](README-power.md) — developers: battery, charging, sleep, and what
  can be switched off (both boards).
- [README-sdcard.md](README-sdcard.md) — developers: the card slot's wiring and how
  to mount it (both boards).
- [README-limitations-errata.md](README-limitations-errata.md) — anyone choosing
  a board: what it can't do, in one page.
- [README-engineering.md](README-engineering.md) — engineers: the reasoning behind
  the numbers, the helper chips' protocols, and how the boards are told apart.
- [schematic-description-7.md](schematic-description-7.md) and
  [schematic-description-7B.md](schematic-description-7B.md) — engineers and LLMs
  without the drawing: each board's schematic, part by part and net by net.

---

## License

MIT — see [LICENSE](LICENSE). `chipguy_ThreadsafeWire` is copied unchanged from
the `chipguy_Board` library.

This library replaces `chipguy_ESP32-S3-Touch-LCD-7_board` and
`chipguy_Waveshare-ESP32-S3-Touch-LCD-7B_board`.
