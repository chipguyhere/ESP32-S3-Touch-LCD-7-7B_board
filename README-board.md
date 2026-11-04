# What's on the board — ESP32-S3-Touch-LCD-7 and -7B

Everything on the board that isn't the screen, the power path or the card slot:
the RS-485 and CAN ports, the headers, the buttons, the two USB-C ports, and which
pins are free for your own hardware. Power is in [README-power.md](README-power.md),
the card slot in [README-sdcard.md](README-sdcard.md). Pin and part references point
into [schematic-description-7B.md](schematic-description-7B.md).

The two boards are one design. Everything below is the -7B, and it holds for the -7
too except where [On the -7](#on-the--7) says otherwise: the I²C header, RS-485, the
termination jumpers, J9 and a few addresses. The -7's parts are in
[schematic-description-7.md](schematic-description-7.md).

## The short version: one free GPIO

The 1024×600 panel uses 21 GPIOs for its data and sync lines, and the rest go to
touch, I²C, the card slot, RS-485, UART and USB/CAN. Only **GPIO6** (on the 3-pin
header) has nothing else on it. Everything below is about getting more than that
by borrowing a pin from a function you don't use.

| You want | Use | Cost |
| --- | --- | --- |
| An I²C sensor | the **I2C header** (H1) and `board.Wire` | nothing — it's a shared bus |
| A button, LED, relay, 1-Wire, analog input | **GPIO6** on the **GPIO header** (J8) | nothing |
| A second UART | the **UART2 header** (H3), SW1 set to UART2 | the USB-UART port stops working |
| An SPI device | GPIO11/12/13 (card-slot lines) + GPIO6 as CS | shares the bus with the card |
| RS-485 | the **RS-485 terminal** (J7) | GPIO15/16 |
| CAN | the **CAN terminal** (J6), `board.selectCan(true)` | the second USB-C port goes dead |

## Headers and terminals

| Header | Silkscreen | Pins | Notes |
| --- | --- | --- | --- |
| H1 | I2C | 3V3, GND, SDA (**GPIO8**), SCL (**GPIO9**) | the board's own I²C bus; 4.7 kΩ pull-ups on board |
| J8 | GPIO | 3V3, GND, **GPIO6** | the one free GPIO; ADC1 channel 5, touch-capable, deep-sleep wake-capable |
| H3 | UART2 | 3V3, GND, RX (**GPIO44**), TX (**GPIO43**) | live only with SW1 on UART2 |
| J7 | RS-485 A B | A, B | through fuses and TVS diodes |
| J6 | CAN L H | CANL, CANH | |
| J9 | POWER GND 5V | GND, 5V | a 5 V **input** (or output — it is the board's 5 V rail) |
| J3 | Battery − + | VBAT, GND | see [README-power.md](README-power.md) |

### An I²C sensor on H1

The bus is already running once the display (or `board.begin()`) has started.
Don't call `Wire.begin()` yourself: use the board's mutex-protected bus, so your
transactions can't collide with touch polling.

```cpp
board.begin();                         // already done if lv_setup.begin() ran
if (board.Wire.probe(0x44)) {          // e.g. an SHT3x
    board.Wire.writeRegister(0x44, 0x24, 0x00);
}
```

Addresses already taken: **0x24** (the I/O expander) and **0x5D** (the GT911 touch
chip). On the -7 the expander takes **0x20–0x27 and 0x30–0x3F** instead of 0x24. Libraries that insist on a `TwoWire&` can be given `board.Wire.raw()`, but
then they bypass the lock — only do that if nothing else touches the bus from
another task. See [README.md](README.md) for the `ThreadsafeWire` details.

### A button or output on GPIO6

```cpp
pinMode(board.SENSOR_PIN, INPUT_PULLUP);   // button to GND
// or: pinMode(board.SENSOR_PIN, OUTPUT);  // LED / relay driver
```

### A second UART on H3

UART0 (GPIO43/44) goes through a switch (U13). With **SW1 on UART1** it reaches the
USB-C port marked UART; with **SW1 on UART2** it reaches H3 instead. So H3 is not an
*extra* UART — it is the same `Serial0`, rerouted. Uploading and the serial monitor
need SW1 back on UART1.

For a second UART *and* the USB console, the only clean option is one direction
on GPIO6 (`Serial1.begin(baud, SERIAL_8N1, 6, -1)` to receive, or `-1, 6` to
send). GPIO15/16 are not an alternative: the RS-485 transceiver drives GPIO15
whenever it is receiving.

### An SPI device

There's no SPI header. The card slot's lines are GPIO11 (MOSI), GPIO12 (SCK) and
GPIO13 (MISO), and a device can share them with GPIO6 as its chip select. If a card
is also in use, run the card in SPI mode on the same bus (see
[README-sdcard.md](README-sdcard.md) — its CS is on the expander), or mount it with
SD_MMC only while the other device is idle. **TODO (bench):** no shared-bus test has
been done.

```cpp
SPI.begin(12, 13, 11, board.SENSOR_PIN);   // SCK, MISO, MOSI, CS
```

There is no physical header for GPIO11–13; you'd solder to the 0 Ω links R98, R99
and R101 next to the slot, or use a microSD breakout in the slot.

## RS-485

An SP3485 transceiver on **TX = GPIO16, RX = GPIO15** (the schematic's net labels
have these the other way round; the board was checked — see
[README-engineering.md](README-engineering.md#rs-485-which-pin-is-tx)). Direction is
switched **automatically** by a small circuit that watches TX, so there is no DE pin
to drive: just use a UART.

```cpp
Serial1.begin(115200, SERIAL_8N1, board.RS485_RX_PIN, board.RS485_TX_PIN);
```

- The transceiver runs from 3.3 V.
- Fail-safe bias resistors (10 kΩ) are on board.
- A **120 Ω terminator** is switched in by the **SW2 DIP switch** (the RS-485 half).
  Turn it on only at the two ends of the bus.
- **Tested to 2.5 Mbaud.** Bench, 2026-10-26: this board against a Waveshare
  ESP32-P4-86-Panel (THVD1406 auto-direction transceiver behind an isolator), both
  ends terminated with 120 Ω. 64-byte packets were sent and echoed back.
  - Every packet was clean at 9600, 115200, 250k, 500k, 1M, 2M (2000 packets) and
    2.5 Mbaud.
  - 3 Mbaud and above failed, whatever the reply delay.
- **Turnaround is not a problem.** The auto-direction circuit holds the driver on
  briefly after each low bit, and the receiver is off meanwhile. Even so, a reply
  sent with no added delay arrived intact at every working speed.

## CAN

A TJA1051 transceiver, reached through the same switch (U9) as the second USB-C
port. CAN and native USB can't be used at the same time.

```cpp
#include "driver/twai.h"
board.selectCan(true);                     // GPIO19/20 -> CAN transceiver
twai_general_config_t g = TWAI_GENERAL_CONFIG_DEFAULT(
    (gpio_num_t)board.CAN_TX_PIN, (gpio_num_t)board.CAN_RX_PIN, TWAI_MODE_NORMAL);
```

- TX = **GPIO20**, RX = **GPIO19**.
- The transceiver's silent-mode pin is tied low, so it is always in normal mode.
- The **120 Ω terminator** is the CAN half of SW2.
- `board.begin()` leaves the switch on **USB**; call `selectCan(true)` before
  starting TWAI.

## The two USB-C ports

| Port | Silkscreen | Goes to | Gives you |
| --- | --- | --- | --- |
| Type_C1 | UART | CH343 USB-UART → UART0 (with SW1 on UART1) | uploads, `Serial0`, auto-reset; **the only port that charges the battery** |
| Type_C2 | USB | ESP32-S3 native USB on GPIO19/20 (unless CAN is selected) | USB-Serial-JTAG, or TinyUSB device modes |

Both ports power the whole board. Uploads work over either (native USB needs the
ESP32 in download mode or a running USB-Serial-JTAG). To use Type_C2 as the serial
console, set **USB CDC On Boot: Enabled** and print to `Serial`; `Serial0` stays on
UART0.

Uploads over the UART port: **use 230400 baud.** 460800 and 921600 corrupted data
on our board's CH343 link (bench, 2026-10-26), while another Waveshare board with
the same CH343 uploaded at full speed on the same computer. **TODO (bench):** a
second 7B unit.

## Buttons and LEDs

| Part | What it does | Usable by a sketch? |
| --- | --- | --- |
| RESET (K1) | resets the ESP32 (EN) | no |
| BOOT (K2) | pulls GPIO0 low | **no while the display runs** — GPIO0 is also a panel data line (G3) |
| POWER LED (red) | on whenever 3.3 V is up | no — hardwired |
| CHARGE / DONE LEDs | driven by the battery charger | no |

There is no user LED and no spare button. The BOOT button still works for its real
job: hold it while pressing RESET to force download mode.

## Pins our libraries already use

| Pins | Used by | For |
| --- | --- | --- |
| GPIO0–3, 5, 7, 10, 14, 17, 18, 21, 38–42, 45–48 | `_display` | RGB panel data, sync, clock |
| GPIO4 | `_display` (touch) | GT911 interrupt / address select |
| GPIO8, 9 | `_board` | I²C (expander, touch, H1) |
| EXIO1–6, PWM, ADC | `_board` | see [README.md](README.md) |

Free for you: GPIO6, and — depending on what you give up — GPIO11/12/13 (card),
GPIO15/16 (RS-485), GPIO19/20 (USB or CAN), GPIO43/44 (USB-UART).

Strapping pins: GPIO0 (BOOT, pulled up), GPIO3 (floating), GPIO45 and GPIO46
(pulled down). All four are panel lines; the display driver owns them after boot.

## On the -7

The ESP32-S3-Touch-LCD-7 (VER1.2 schematic) differs from the -7B here. Part and
section references are to [schematic-description-7.md](schematic-description-7.md).

| What | -7 | Why it matters |
| --- | --- | --- |
| I²C header H1 | silkscreen **I2C VCC GND SDA SCL**; its own supply `I2C_VCC`, set by a 0 Ω resistor at position **H2** — fitted to **3V3** as shipped, with a 5V position — through a level shifter (§1.11) | works as a 3.3 V header as shipped. Don't move the resistor to 5V: as drawn, that may lift the board's 3.3 V I²C bus. **TODO (bench).** |
| Expander addresses | the CH422G answers **0x20–0x27 and 0x30–0x3F** | rules out PCF8574, MCP23017, TCA9554 and similar parts on the -7 |
| GPIO6 header J8 | silkscreen **Sensor AD: 3V3 GND AD** | same pin (GPIO6), different label |
| RS-485 | SP3485 on **5 V**; TX on GPIO16 switches the driver through a transistor; RX on GPIO15 (§1.13) | same `Serial1.begin(..., RS485_RX_PIN, RS485_TX_PIN)`. **Hazard, measured:** GPIO15 sits at 5 V whenever the port is receiving — see [errata 21](README-limitations-errata.md) and below. Speed not tested. |
| Terminators | two removable jumpers, **J4** (RS-485) and **J5** (CAN), silkscreen **120R / NC** — the -7B has a mini DIP switch (SW2) instead | move the jumper to **120R** at the two ends of a bus |
| J9 (5 V terminal) | not fitted | |
| CAN | as -7B (bench, 2026-11-03: 500 kbit/s to a -7B, no lost frames) | |
| Panel clock R27 | 100 Ω, no capacitor | nothing to do |
| Backlight | on/off only; the dimming option's pad P1 is not on any pin (§1.8) | `setBacklight()` treats 1–100 as on |

### -7: GPIO15 and the 5 V receiver output

The -7's RS-485 transceiver runs from 5 V, and while it is receiving — the normal
idle state, even with nothing plugged in — its output holds GPIO15 at 5 V (measured,
2026-11-03). Holding **GPIO16 low** switches the chip to transmit, which turns its
receiver output off and lets GPIO15 fall back to 3.3 V (also measured).

**The library does this for you.** On a -7, `board.begin()` (and so
`lv_setup.begin()`) holds GPIO16 low. You only need to know three things:

- **Using RS-485:** just open the port. `Serial1.begin(115200, SERIAL_8N1,
  board.RS485_RX_PIN, board.RS485_TX_PIN)` takes GPIO16 over, and the port works
  normally. GPIO15 is then at 5 V whenever the port is receiving; on this board
  that can't be avoided while RS-485 is in use.
- **The cost:** while the guard is on, the transceiver drives the RS-485 line —
  about 1 mA with nothing attached.
- **On a live RS-485 network without opening the port,** the guard would hold
  the whole bus in one state. Turn it off before the board starts:

```cpp
board.setRs485Guard(false);   // before lv_setup.begin() / board.begin(), or any time after
lv_setup.begin();
```

or build with `-DLCD7_7B_RS485_GUARD=0`. On the -7B both are no-ops: its
transceiver runs at 3.3 V. The permanent fix is a hardware change; see
[README-engineering.md](README-engineering.md#rs-485-113).
