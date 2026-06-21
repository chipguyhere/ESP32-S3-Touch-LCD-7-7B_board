# chipguy_ESP32-S3-Touch-LCD-7_board

A small **board-support** library for the Waveshare **ESP32-S3-Touch-LCD-7**. It
owns the two things on this board that every other driver needs to share: the
**I²C bus** and the **CH422G I/O expander**.

You usually don't use this library directly. It is a **dependency** of
**`chipguy_ESP32-S3-Touch-LCD-7_display`**, which drives the display and touch
through it. Install it alongside that library and forget about it — unless you
want to put your own I²C devices on the shared bus, or toggle one of the spare
expander pins yourself.

---

## Why this library exists

On the ESP32-S3-Touch-LCD-7, several control lines are **not** wired to ESP32
GPIOs. They hang off an onboard **CH422G I²C expander**:

| Line             | CH422G pin | Purpose                          |
|------------------|------------|----------------------------------|
| LCD reset        | EXIO3      | resets the ST7262 RGB panel      |
| LCD backlight    | EXIO2      | backlight enable (on/off)        |
| Touch reset      | EXIO1      | resets the GT911, latches 0x5D   |

That expander, plus the GT911 touch controller and any other I²C peripherals,
all share **one I²C bus** (`SDA = GPIO8`, `SCL = GPIO9`). Two problems follow,
and this library solves both:

1. **The bus must be shared safely.** It is wrapped in a mutex-protected
   `ThreadsafeWire`, so multiple FreeRTOS tasks and drivers can use it at once
   without corrupting each other's transactions.
2. **The CH422G is not a normal register-mapped expander.** Instead of a register
   index, it responds to a handful of fixed I²C "command" addresses, each a
   single-byte write (`WR_SET` 0x24 to enable the IO0–7 outputs, `WR_IO` 0x38 to
   drive them, …). This library hides that quirk behind named, device-specific
   methods.

---

## What it provides

A single class, `ESP32S3_Touch_LCD_7_Board`:

```cpp
#include <ESP32S3_Touch_LCD_7_Board.h>

ESP32S3_Touch_LCD_7_Board board;

void setup() {
    board.begin();              // bring up the I²C bus + put the CH422G in a known state
}
```

- **`begin()`** — brings up the shared I²C bus and puts the CH422G in its known
  power-on state (IO0–7 enabled as outputs; reset lines de-asserted; backlight
  off so the display can draw a first frame before it is revealed). Idempotent:
  the display and touch drivers both call it, only the first does the work.
- **`Wire`** — the shared, mutex-protected I²C bus (a `ThreadsafeWire`). Use it
  for your own I²C devices; hold a `ThreadsafeWire::Guard` across multi-step
  (repeated-start) transactions.
- **Device-specific control:**
  - `lcdReset()` — pulse the panel reset (EXIO3) low→high.
  - `lcdBacklight(bool on)` — switch the LCD backlight (EXIO2). On/off only;
    this pin has no PWM.
  - `touchReset()` — hold the GT911's INT (GPIO4) low while pulsing its reset
    (EXIO1), which latches the controller at I²C address 0x5D, then release INT.
- **`exioWrite(io, level)`** — escape hatch to toggle any CH422G output pin
  (`io` is 0–7 = IO0–IO7). EXIO4 (SD_CS) and EXIO5 (USB_SEL) and the other
  unused pins are yours to drive.

### Using the shared bus for your own I²C devices

```cpp
ESP32S3_Touch_LCD_7_Board board;

void setup() {
    board.begin();

    // Single-shot helpers (mutex-protected internally):
    board.Wire.writeRegister(0x68, 0x6B, 0x00);     // e.g. wake an MPU-6050
    uint8_t whoami;
    board.Wire.readRegister(0x68, 0x75, whoami);

    // Multi-step transaction? Hold the bus for the whole sequence:
    {
        ThreadsafeWire::Guard g(board.Wire);
        board.Wire.beginTransmission(0x68);
        board.Wire.write(0x3B);
        board.Wire.endTransmission(false);          // repeated start
        board.Wire.requestFrom((uint8_t)0x68, (size_t)6);
        // ...read 6 bytes...
    }
}
```

---

## Installation

Download this library as a ZIP from GitHub (**Code ▸ Download ZIP**) and move the
unzipped folder into your Arduino **libraries** directory
(`~/Documents/Arduino/libraries/` on macOS/Linux,
`Documents\Arduino\libraries\` on Windows), then restart the Arduino IDE.
Install **`chipguy_ESP32-S3-Touch-LCD-7_display`** the same way — it depends on
this library.

This library is self-contained: its only dependency is the Arduino core's
`Wire`.

---

## License

MIT — see [LICENSE](LICENSE).
