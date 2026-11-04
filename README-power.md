# Power — ESP32-S3-Touch-LCD-7 and -7B

How the board is powered, what the battery reading means, and what can and can't
turn things off. Part references point into
[schematic-description-7B.md](schematic-description-7B.md); the reasoning behind the
inferred figures is in [README-engineering.md](README-engineering.md#power).

The answers below are for the -7B. The -7 has the same charger, regulator, USB
ports and lack of a power switch; where it differs — no battery reading, no
dimming, no J9, the panel bias's default — see [On the -7](#on-the--7) at the end,
and [schematic-description-7.md](schematic-description-7.md) for its parts.

## Power characteristics at a glance

### Power sources

**Can this board be powered by USB?** **Yes**, from either USB-C port, and the
whole board runs from either one. They are not equal: only the port marked
**UART** charges the battery and powers the USB-UART chip. The port marked **USB**
feeds the 5 V rail directly and bypasses the charger.

**Can this board run on battery power?** **Yes.** A single lithium cell plugs into
J3 (silkscreen "Battery − +"). A charge-and-boost chip (U4, CS8501) charges it and
boosts it to 5 V for the rest of the board. **TODO (bench):** the connector type
and pitch; cell chemistry is assumed Li-ion / LiPo 4.2 V from the 3.7 V-class
charger (inferred).

**Can the board recognize power from another input?** **No.** J9 ("POWER GND
5V") is a bare 5 V input on the same rail as the USB port's VBUS. Nothing reports
its presence to software.

**Can software tell which source is powering the board right now?** **No.**
There is no sense line from either USB port or J9 to a GPIO or the expander.
Indirect hints only: the battery reading wanders around 4.1–4.8 V when no cell is
fitted (see below), and native USB on the USB port can see a host enumerate.

**Can we run only on external power and keep the battery charged?** **No.** The
charger switches to battery-boost on its own when USB goes away, and there is no
switch in the battery path.

### Power management chip and protection

**Does this board have a PMIC?** **No.** A single autonomous charger-plus-boost
chip, U4 (marked **CS8501** on the schematic), with no connection to the ESP32 or
the expander. Its two status outputs drive the CHARGE and DONE LEDs only. A 3.3 V
LDO (U5, SGM2212) makes the logic rail from 5 V.

**Does the board stop a lithium cell from discharging too far?** **Unknown.**
Charger-boost chips of this kind usually stop boosting below about 3 V, but no
datasheet for the CS8501 was found. **TODO (datasheet / bench):** the cut-off
voltage, and whether it is a clean stop. Until then, use a cell **with its own
protection PCB**, and have the sketch own the graceful floor: watch
`board.batteryVoltage()` and sleep before it gets low.

**Can software control charging?** **No.** Charging is fixed at about **580 mA**
(the schematic's table for the 1.2 kΩ PROG resistor, R64). The termination voltage
comes from the chip — **TODO (datasheet)**, presumably 4.2 V.

### Battery measurement

**Can we read the battery level?** **Yes — a voltage.** VBAT goes through a
20 kΩ / 10 kΩ divider (R18/R19) to the expander's ADC pin. `board.batteryVoltage()`
returns volts at the cell; `board.batteryRaw()` the raw reading. The expander
reports it on a 10-bit scale against 3.3 V (Waveshare's demo), so one count is
about **9.7 mV at the cell**. There is no fuel gauge and no percentage.

**What is the practical resolution?** **Unknown with a cell fitted.** **TODO
(bench):** measure the spread with a real cell. Without a cell the reading jumps by
±0.35 V between samples, but that is the charger hunting, not ADC noise. Average
several readings; expect the result to be good for a battery icon and a low-battery
cut-off, not for coulomb counting.

**Can we tell whether charging is active, or whether charging power is
available?** **No** to both. The charger's CHRG and STDBY outputs drive LEDs only.

**Can the board detect whether a battery is present?** **No.** With no cell
fitted, `batteryVoltage()` reads a plausible-looking **4.1–4.8 V** that changes from
one reading to the next (bench, 2026-10-26, USB on the UART port). That is the
charger driving the empty connector, not a battery. A reading that jumps around
like that is the only hint.

**Does the reading mean the same on USB and on battery?** **No.** On the UART port
the charger is pushing the cell up, so the reading is higher than the cell's
resting voltage. Estimate state of charge only while running on battery.

### Indicators

**Power light?** **Yes, not controllable.** The red POWER LED is hardwired to the
3.3 V rail, so it is on whenever the board is, and draws current in every state —
**TODO (bench):** how much.

**Battery / charge lights?** **Yes, not controllable.** CHARGE (green) and DONE
(red) are driven by the charger. They are powered from the UART port's VBUS, so
they are dark when the board runs on battery or on the USB port.

### Shutting down, sleeping, waking

**Can the board shut itself off?** **No.** Nothing can cut the battery, the boost
output or the 5 V rail. The deepest state is ESP32 deep sleep with the board's
rails still up.

**Can it shut off when USB or DC is present?** **No** — see above. The library
offers nothing in place of "off"; use deep sleep.

**Can it power down and still wake on a timer?** **No** rail-off state exists.
ESP32 deep sleep with a timer wake works as on any ESP32-S3.

**Which interrupts can wake it?** GPIO0 (BOOT button), GPIO4 (touch interrupt) and
GPIO6 (J8) are deep-sleep wake-capable. The I/O expander and touch chip stay
powered while the ESP32 sleeps (3.3 V is always on).

**Is there an IMU?** **No.**

**Can the touchscreen practically wake the board?** **Probably.** The GT911's
interrupt is on GPIO4, a wake-capable pin, and the GT911 stays powered. **TODO
(bench):** confirm that a touch pulls INT low with the display off and wakes the
ESP32 via `ext0`.

**Does anything have to be done at boot to stay powered?** **No.**

### The power button

**Is there a power button?** **No.** There is RESET (on EN) and BOOT (GPIO0) only.

**If there's no hard-off, how do you get out of a hang on battery?** Press
**RESET**. To stop the board entirely, unplug the battery.

### Other buttons

**Is BOOT available as a user button?** **No, not while the display runs.** GPIO0
is also a panel data line (G3), driven by the RGB peripheral. Its only use is
forcing download mode: hold BOOT, press RESET.

**Is there a reset button?** **Yes**, on EN. Not software-visible.

### Peripheral power switches

| Rail | Feeds | Switch | Default after `board.begin()` | API |
| --- | --- | --- | --- | --- |
| Backlight boost (U3) | panel backlight | EXIO2 (enable) + expander PWM (dimming) | off until the display reveals the first frame | `board.lcdBacklight()`, `board.setBacklightBrightness()` |
| Panel bias (U2: AVDD, VGH, VGL, VCOM) | panel glass | EXIO6 | on | `board.lcdPower()` |

**What cannot be powered off:** the 3.3 V rail and everything on it — the ESP32
module, the I/O expander, the touch chip, the panel's logic supply, the card slot,
RS-485 and the POWER LED. The 5 V rail and the CAN transceiver. The battery divider
(about 0.14 mA from the cell, always).

### Power consumption

| State | Battery current | How measured |
| --- | --- | --- |
| Running, backlight 100 % | **TODO (bench)** | — |
| Running, backlight off (`lcdBacklight(false)`) | **TODO (bench)** | — |
| Running, backlight and panel bias off | **TODO (bench)** | — |
| ESP32 deep sleep, backlight and panel bias off | **TODO (bench)** | — |
| "Off" (nothing can be turned off; see above) | same as deep sleep | — |

Lower bound, inferred: the battery divider alone draws ~0.14 mA, the POWER LED
around 1 mA, the boost converter its own quiescent current (unknown), and the
expander MCU runs continuously. Expect deep sleep to be measured in milliamps,
not microamps. **TODO (bench):** whether the CS8501's boost shuts itself off at
light load (some charge-and-boost chips do), which would make deep sleep
unexpectedly turn the board off.

### Recommendations

**`setup()` order:** nothing special. `board.begin()` (or `lv_setup.begin()`, which
calls it) leaves the panel bias on and the backlight off until the first frame is
drawn.

**Always-on, on USB, with a battery you don't need:** nothing can be configured —
charging is autonomous and the battery takes over whenever USB drops. If you don't
want the cell cycled, remove it.

**Battery fitted but run as USB-only:** not possible in software; same answer.

**Battery as the main source:**
- Average `board.batteryVoltage()` over several readings while on battery.
- Pick a floor (3.4–3.5 V for a Li-ion cell is a safe start) and go to deep sleep
  with the backlight and panel bias off below it.
- The library does nothing automatically. It reads the voltage and switches the
  rails; the policy is yours.

## On the -7

From the VER1.2 schematic ([schematic-description-7.md](schematic-description-7.md));
nothing here has been measured.

| Question | -7 answer |
| --- | --- |
| Power sources | the same two USB-C ports and battery connector J3, with the same roles; **no J9** 5 V terminal |
| Charger | the same CS8501, 580 mA, LEDs CHARGE / DONE |
| **Can we read the battery level?** | **No.** A 200 kΩ / 100 kΩ divider exists, but it goes only to test pad TP1, and the CH422G expander has no ADC. `board.batteryVoltage()` returns NAN and `batteryRaw()` 0. A sketch that needs the level must wire TP1 to a spare ADC pin (GPIO6, ADC1_CH5, on J8), reading VBAT ÷ 3. **TODO (bench).** |
| Backlight | **on/off only** (EXIO2). No dimming input reaches a pin: `setBacklightBrightness()` returns false. |
| Panel bias (EXIO6) | **off at power-up** (pull-down), on once `board.begin()` runs — the opposite default to the -7B, which matters only before `begin()` |
| Always drawing | the battery divider, about **14 µA** (the -7B's draws ~140 µA); the POWER LED; the CH422G |
| Power consumption | **TODO (bench)**, as on the -7B |
