# microSD card slot — ESP32-S3-Touch-LCD-7 and -7B

The chipguy libraries for this board don't implement a filesystem for the card
slot. This file gives you what you need to bring up the driver of your choice.
Pin and net references come from
[schematic-description-7B.md](schematic-description-7B.md).

## Interface and pinout

The slot (SD1) is wired for **1-bit SD mode** or **SPI mode**. D1 and D2 are not
connected, so 4-bit SD mode is impossible.

| Card pin | SD-mode role | SPI-mode role | Goes to | Net |
| --- | --- | --- | --- | --- |
| 5 CLK | CLK | SCK | **GPIO12** (via R99, 0 Ω) | `IO12` |
| 3 CMD | CMD | MOSI | **GPIO11** (via R98, 0 Ω) | `IO11` |
| 7 D0 | D0 | MISO | **GPIO13** (via R101, 0 Ω) | `IO13` |
| 2 CD/D3 | D3 (must sit high) | CS | **EXIO4** on the I²C expander (via R92, 0 Ω) | `EXIO4` |
| 8 D1, 1 D2 | — | — | not connected | |
| 9 card detect | — | — | tied to GND | |

- **Pull-ups:** CLK, CMD, D0 and D3 each have a 10 kΩ pull-up to 3.3 V on the
  board (R88–R91). Nothing to enable in software.
- **No card detect.** The detect contact is grounded, so a sketch learns that
  the card is missing only when mounting fails.

## Which mode to use

**1-bit SD_MMC** is what Waveshare's demo uses, and the natural fit: it needs
only the three real GPIOs, and D3 just has to stay high, which the pull-up and
`board.begin()` (EXIO4 high) both see to.

```cpp
#include <SD_MMC.h>
board.begin();                          // leaves EXIO4 (D3) high
SD_MMC.setPins(12, 11, 13);             // CLK, CMD, D0
if (!SD_MMC.begin("/sdcard", true)) {   // true = 1-bit mode
    Serial0.println("No card, or mount failed");
}
```

**TODO (bench):** confirm this mounts a card on this board with our libraries
loaded.

**SPI mode** works with any SPI driver that lets you supply chip-select
yourself, because CS is an expander pin, not a GPIO: call
`board.sdChipSelect(true / false)` around each transaction, or hold it selected
for a card that is alone on its bus. The stock Arduino `SD` library wants a GPIO
number for CS, so it is the awkward choice here. Each CS change is an I²C write,
which costs time on every transaction — one more reason to prefer SD_MMC.

## Bus sharing

**Not shared.** GPIO11/12/13 go only to the card slot. The display, touch and
expander do not use them.

## What our libraries already occupy

| Peripheral | Used by | For |
| --- | --- | --- |
| LCD_CAM (RGB mode) + one GDMA channel | `_display` | the 1024×600 panel |
| I²C0 (`Wire`, GPIO8/9) | `_board` | the expander and the GT911 touch chip |
| SPI2_HOST, SPI3_HOST | — | **free** |
| SDMMC host (both slots) | — | **free** |

## Strapping and loaded pins

GPIO11, 12 and 13 are not strapping pins, so the card may be present at reset.
D3 is held high by its pull-up even before the expander is set up.

## On the -7

The same slot, the same wiring and the same pins (VER1.2 schematic,
[schematic-description-7.md §1.12](schematic-description-7.md#112-tf--microsd-sd1)):
SPI or 1-bit SD on GPIO11/12/13, CS on expander line EXIO4, no card detect, no 4-bit
mode. The only difference is a 100 Ω series resistor between the CH422G and the CS
line, which changes nothing for a sketch. Not yet bench-tested on a -7.
