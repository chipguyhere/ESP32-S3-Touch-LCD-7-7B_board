# Limitations and errata — ESP32-S3-Touch-LCD-7 and -7B

What this board can't do, does wrong, or might be stressed by, read in a minute
before you commit to it. Each item links to where the detail lives.

Items 1–16 were written for the **-7B** (Waveshare schematic dated 2025-05-13, one
unit tested 2026-10-26). They hold for the **-7** too (VER1.2 schematic; bench
tests 2026-11-03 on a VER1.1 board), except **5** (the -7 has no battery reading at all, item 18),
**10** (the -7 took 460800 baud) and **13** (the -7's R54 is a pull-down: no
overvoltage). Items 17–23 are the -7's own.

1. **Only one free GPIO.** The 1024×600 panel takes 21 pins, and every other GPIO
   goes to touch, I²C, the card slot, RS-485, UART or USB/CAN. GPIO6 on the 3-pin
   header is the only pin with nothing on it; anything more means giving up a
   function. → [README-board.md](README-board.md#the-short-version-one-free-gpio)

2. **The display keeps a CPU core busy.** The panel has no frame memory, so the
   ESP32 re-sends all 1.2 MB from PSRAM 32.7 times a second. That runs on core 1,
   along with copying what LVGL draws into the picture: together, roughly 25–90 %
   of core 1 depending on how much is moving. Running Arduino on core 0 leaves
   LVGL a core of its own. Lower refresh rates free CPU, but the panel's black
   areas visibly flicker below about 30 Hz.
   → [display README-engineering.md](../chipguy_Waveshare-ESP32-S3-Touch-LCD-7-7B_display/README-engineering.md#the-shape-of-it)

3. **Nothing can turn the board off.** There is no power switch, no PMIC and no
   load switch. On battery the board runs until the cell is unplugged or the
   charger's own cut-off (if any) acts, and deep sleep still powers the rails, the
   expander and the POWER LED. Use a cell **with a protection PCB**.
   → [README-power.md](README-power.md#shutting-down-sleeping-waking)

4. **Only the UART USB port charges the battery.** The USB port and the 5 V header
   feed the 5 V rail directly, bypassing the charger, and the USB-UART chip is
   unpowered unless the UART port is plugged in.
   → [README-power.md](README-power.md#power-sources)

5. **The battery reading can't tell "no battery" from "full".** With no cell
   fitted it reports a plausible 4.1–4.8 V that changes between readings. There is
   also no charge-status signal; the CHARGE and DONE LEDs are hardwired.
   → [README-power.md](README-power.md#battery-measurement)

6. **The BOOT button is not a user button.** GPIO0 is also a panel data line, so
   while the display runs it can't be read. There is no other user button and no
   user LED. → [README-board.md](README-board.md#buttons-and-leds)

7. **Native USB and CAN share GPIO19/20.** A switch picks one; using CAN turns off
   the USB port's data lines (it still powers the board).
   → [README-board.md](README-board.md#can)

8. **The UART header is not a second UART.** H3 and the USB-UART chip are two
   routes for the same UART0, chosen by slide switch SW1. On UART2, uploads and the
   serial monitor stop working. → [README-board.md](README-board.md#a-second-uart-on-h3)

9. **Slide switch SW1 must be on UART1 to upload.** On UART2, esptool gets no
   response at all, which looks exactly like a dead board.
   → [README-board.md](README-board.md#the-two-usb-c-ports)

10. **Uploads failed above 230400 baud on the UART port.** On the tested unit,
    460800 and 921600 corrupted data every time. A different Waveshare board with
    the same CH343 chip uploaded at full speed on the same computer and driver, so
    this points at the 7B's UART path rather than the host (inferred). **TODO
    (bench):** a second 7B unit, and a scope on GPIO43.
    → [README-board.md](README-board.md#the-two-usb-c-ports)

11. **The schematic's RS-485 labels are swapped.** The nets marked `RS485_TXD` /
    `RS485_RXD` (and Waveshare's pin table) say TX is GPIO15; it is actually
    **GPIO16**, and RX is GPIO15 (checked on the bench).
    → [README-engineering.md](README-engineering.md#rs-485-which-pin-is-tx)

12. **No card detect, and no 4-bit SD.** The slot's detect contact is grounded and
    D1/D2 are unwired, so the card runs in 1-bit SD or SPI mode, and a missing card
    shows only as a failed mount. → [README-sdcard.md](README-sdcard.md)

13. **Possible overvoltage on an expander pin.** **TODO (datasheet):** R54 pulls
    EXIO6 (the panel-bias enable) up to **5 V** through 10 kΩ, while the expander
    chip (likely a CH32V003, inferred) runs at 3.3 V. If that pin isn't 5 V-tolerant,
    up to about 0.17 mA flows into its clamp diode. It works on the tested unit;
    nothing can be done about it in software.
    → [schematic-description-7B.md §1.6](schematic-description-7B.md#16-io-expander-u10)

14. **The USB-UART chip is half-powered when its port is unplugged.** **TODO
    (datasheet / bench):** the CH343's I/O supply comes from the board's 3.3 V while
    its main supply comes from the UART port. What it does to UART0's lines when
    unpowered is unknown; Serial0 output on battery or the USB port is untested.
    → [schematic-description-7B.md §4](schematic-description-7B.md#4-what-the-sheet-does-not-say)

15. **PSRAM is one shared bus, and the display needs it continuously.** With
    Arduino on core 0, a sketch that moves a lot of data through PSRAM (scrolling
    a large image, big buffers) competes with the display. The screen then shows
    brief bands of rows out of place: up to about 15 a second while scrolling a
    picture held in PSRAM. With Arduino on core 1 the two take turns and the
    bands don't appear. Keep PSRAM-heavy sketches on core 1.
    → [display README-engineering.md](../chipguy_Waveshare-ESP32-S3-Touch-LCD-7-7B_display/README-engineering.md#why-core-1-for-all-the-psram-work)

16. **Drawing is done in software, and wide motion is slow at full resolution.**
    LVGL draws every pixel with the CPU. Scrolling a list of text across much of
    the screen runs at about 7–11 frames a second at 1024×600 with Arduino on
    core 0, and slower on core 1. The optional half-resolution mode (512×300,
    scaled up smoothly) brings that to about 19–28, at the cost of softer small
    text.
    → [display README-engineering.md](../chipguy_Waveshare-ESP32-S3-Touch-LCD-7-7B_display/README-engineering.md#where-lvgls-time-goes)

## ESP32-S3-Touch-LCD-7, and both boards

17. **-7: the panel loses lock below about 12 MHz pixel clock.** Its ST7262
    controller then shows a built-in self-test colour cycle instead of the
    picture, and 12 MHz itself only works from some cold starts. The driver
    raises any request below 13 MHz to 13 MHz (about 32 Hz refresh), so the -7
    can't trade refresh rate for CPU time the way the -7B can.
    → [display README-engineering.md](../chipguy_Waveshare-ESP32-S3-Touch-LCD-7-7B_display/README-engineering.md#refresh-rate)

18. **-7: the backlight is on or off, and there is no battery reading.** Its
    CH422G expander has no PWM and no ADC. `setBacklight()` treats any level
    above 0 as full on; `setBacklightBrightness()` and `batteryVoltage()` return
    `false` / NAN. The battery still charges, and a ÷3 divider sits on test pad
    TP1; wiring TP1 to GPIO6 would give a sketch the voltage.
    → [README-power.md](README-power.md#on-the--7)

19. **-7: the expander's panel-reset resistor may not be fitted.** R112, between
    the CH422G and the panel's RESET line, is drawn as "NC/100R". If it is NC,
    `lcdReset()` does nothing and the panel resets only at power-up through its
    pull-up. Nothing visible depends on it so far. **TODO (bench).**
    → [schematic-description-7.md §1.6](schematic-description-7.md#16-io-expander-u10)

20. **-7: the CH422G takes sixteen I²C addresses.** It answers every address
    in 0x20–0x27 and 0x30–0x3F (bench, 2026-11-03), not just its four command
    addresses, so a part of your own can't use those on the -7 — that rules
    out the usual PCF8574 / MCP23017 / TCA9554 range. The -7B's expander
    answers only 0x24. `begin()` tells the boards apart by 0x38, so a part at
    0x38 on a -7B would make it misidentify the board.
    → [README-engineering.md](README-engineering.md#telling-the-two-boards-apart)

21. **-7: the RS-485 transceiver puts 5 V on GPIO15.** The SP3485, a 3.3 V
    part, is powered from 5 V on the -7, and its receiver output drives GPIO15
    directly (the 4.7 kΩ to 3.3 V is only a pull-up). In receive — which is
    whenever RS-485 isn't transmitting, including with nothing connected —
    GPIO15 sits at 5 V, above the ESP32-S3's 3.3 V rating. **Measured** on a
    VER1.1 board (2026-11-03): 5 V on U6 pin 8 (VCC) and pin 1 (RO), and 5 V at
    the module's GPIO15. The VER1.2 schematic draws the same circuit, so V1.2 is
    expected to match (inferred; no V1.2 board measured).
    Software can't remove it, only move it: holding GPIO16 low disables the
    receiver and brings GPIO15 back to 3.3 V (measured). **`board.begin()` does
    that by default on the -7**; opening the RS-485 port lifts it, and
    `setRs485Guard(false)` turns it off for a board on a live bus — see
    [README-board.md](README-board.md#-7-rs-485-puts-5-v-on-gpio15). **Using**
    RS-485 on a -7 always exposes GPIO15; fix the hardware first (same section). Don't wire anything else to
    GPIO15, and treat long-term use of the -7's RS-485 port as at your own risk;
    the lasting fix is a hardware change (power U6 from 3.3 V, or add a series
    resistor into GPIO15).
    → [schematic-description-7.md §1.13](schematic-description-7.md#113-rs-485)

22. **-7: the I²C header's 5 V option may lift the board's I²C bus.** A 0 Ω
    resistor at position H2 powers the header from 3.3 V (as shipped) or, moved,
    5 V, through a two-MOSFET level shifter. Only a board that has been reworked
    to 5 V is affected. As drawn, with H2 on 5 V the shifter's body diodes point from the
    5 V side into the 3.3 V bus shared by the ESP32, the expander and touch.
    Inferred from the symbol's pin roles; **TODO (bench):** measure GPIO8/9 idle
    with H2 on 5 V. Leave the resistor on 3V3 until then.
    → [schematic-description-7.md §1.11](schematic-description-7.md#111-the-ic-bus)

23. **-7: the panel bias is off until the expander is set up.** Its enable has a
    pull-down (the -7B's has a pull-up), so the panel's high-voltage rails stay
    off from power-up until `board.begin()` drives EXIO6 high. Harmless with this
    library, which always does; a sketch that skips `board.begin()` gets no
    picture.
    → [schematic-description-7.md §1.9](schematic-description-7.md#19-panel-bias-panel_voltage)
