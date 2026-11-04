# Engineering notes — ESP32-S3-Touch-LCD-7 / -7B board

> The sections after [Telling the two boards apart](#telling-the-two-boards-apart)
> are the **-7B**. The -7's differences are derived in [The -7's
> circuits](#the--7s-circuits) at the end, from
> [schematic-description-7.md](schematic-description-7.md).

The reasoning behind the figures and verdicts in [README-power.md](README-power.md),
[README-board.md](README-board.md) and [README-sdcard.md](README-sdcard.md). The
complete reading of the schematic — every part, pin and net — lives in
[schematic-description-7B.md](schematic-description-7B.md); this file keeps only the
analysis and cites it by section (§). The display driver's internals are in the
display library's
[README-engineering.md](../chipguy_Waveshare-ESP32-S3-Touch-LCD-7-7B_display/README-engineering.md).

## Telling the two boards apart

The -7's facts here are from its V1.2 schematic (Altium, 2025-11-06, netlist
read from the PDF's bookmarks) and Waveshare's -7 SDK. The -7 and -7B share the ESP32-S3 module's wiring for the RGB panel (identical
data/sync GPIOs), I²C (SDA 8, SCL 9), the GT911 (INT on GPIO 4, reset on
expander pin 1) and the expander pin numbers for backlight (2), panel reset (3)
and SD CS (4). They differ in the expander itself:

| | -7 | -7B |
| --- | --- | --- |
| Part | CH422G | U10, a CH32V003-class MCU with Waveshare firmware |
| Addressing | one I²C address per function: 0x24 system, 0x38 IO write, 0x26 IO read, 0x23 OC write | 0x24 only, register-style (below) |
| PWM / ADC | none | backlight PWM, battery ADC |
| Pin 5 | USB / CAN switch U9, R117 pull-down | the same |
| Pin 6 | panel supply enable (U2 pin 4, R54 pull-down) | the same |
| Battery | charger U4 on VBAT, no reading | charger + ADC reading |

**Both answer at 0x24, and a wrong write there is harmful.** The -7B's
`begin()` writes `0x03, latch` and `0x02, 0xFF` to 0x24. On a CH422G, 0x24 is
the system register: bit 0 IO_OE, bit 1 A_SCAN, bit 2 OD_EN, bit 3 SLEEP
(Waveshare's `esp_io_expander_ch422g.c`). `0x03` would start its LED-scan
mode, and `0x02` would keep scanning with IO0–7 switched to inputs, releasing
the panel and touch resets. The other way round, the -7's `0x01` to the -7B is
a register pointer with no data, which should be harmless.

So `begin()` probes before writing: an address-only transaction (START,
address + W, STOP — no data byte) to **0x38**, the CH422G's IO-latch *write*
address. An ACK means -7. Otherwise an ACK at 0x24 means -7B, and neither means
no supported board (`begin()` returns false). 0x26 isn't used because it is the
CH422G's *read* address, so a write probe there might be NACKed and send a -7
down the -7B path. The order is fail-safe: anything that answers 0x38 is
treated as a -7, never given the 7B's writes. `LCD7_7B_FORCE_MODEL` (7 or 0x7B)
skips the probe.

The probe changes no state, so it doesn't disturb the -7B expander's
reset-surviving registers (next section).

Bench, 2026-11-03, an address-only scan of 0x08–0x77 on one board of each:

| Board | Answers | GT911 config |
| --- | --- | --- |
| -7 | 0x20–0x27, 0x30–0x3F, 0x5D | 800×480 |
| -7B | 0x24, 0x5D | 1024×600 |

So the CH422G answers two whole address blocks, and the -7B's expander only
0x24. One binary then ran on both, picked the right panel each time, and drove
CAN between the two at 500 kbit/s through `selectCan(true)`, with no lost
frames (2026-11-03). A cross-check that needs no expander is the GT911's
configured resolution (registers 0x8048–0x804B), which reads 800×480 or
1024×600 — not used yet, because the GT911 must be out of reset first, and
resetting it goes through the expander.

## The I/O expander (-7B)

U10 is a 20-pin microcontroller running Waveshare firmware, not a dedicated
expander chip. Its pinout (PA/PC/PD ports, PD7/NRST, SWDIO on PD1, I²C on PC1/PC2)
matches a WCH CH32V003F4U6 (inferred, §1.6). Programming access is a single SWDIO
test pad, so the firmware is effectively fixed.

Protocol, from Waveshare's demo and confirmed on the bench (2026-10-26):

| Reg | Access | Meaning | Bench |
| --- | --- | --- | --- |
| 0x01 | read | — | reads **0xAA**; plausibly a firmware ID |
| 0x02 | write/read | direction, bit = 1 → output | reads back |
| 0x03 | write/read | output latch | reads back |
| 0x04 | read | pin levels | follows the outputs; unconnected inputs read 1 |
| 0x05 | write/read | PWM duty 0–255 (backlight) | reads back; 0 after power-up |
| 0x06 | read | ADC, 16-bit little-endian, 10-bit range | |
| others | read | — | 0xFF |

Every access is a register write (`reg, value`) or a write-then-read. The registers
**survive an ESP32 reset**, because U10 runs from the same 3.3 V rail but has no
reset line from the ESP32 (its NRST pin goes nowhere). So a sketch that boots can
find the backlight off, a reset line held, or CAN selected from the previous run.
That's why `begin()` writes the full output byte and the direction byte rather than
assuming power-on defaults. It writes the **output latch first, then the
direction**, so that a pin switching from input to output comes up at the intended
level rather than at whatever the latch held.

`EXIO_RESET`, `EXIO0` and `EXIO7` are net labels with nothing else on them (§1.6).

## Power

### Which port does what (§1.2)

- **Type_C1 (UART)** VBUS → `VUART`. Two PNP-controlled P-MOSFET ideal diodes take it
  onward: Q5 (with Q7/Q8) into `VBUS`, and Q10 (with Q11/Q12) into `5V`. `VBUS`
  feeds the CH343, the charge LEDs and the charger's input, and also `5V` through
  the Schottky D11.
- **Type_C2 (USB)** VBUS and **J9** sit directly on `5V`.
- Nothing carries `5V` back into `VBUS` (D11 and both ideal diodes point away from
  it), so **only Type_C1 charges**, and the CH343 is dead without Type_C1.
- Type_C2's VBUS pins are the `5V` rail itself. With the board on battery or on
  Type_C1, 5 V appears on Type_C2's VBUS. A USB-C host's own VBUS would then face
  ours. USB-C hosts shouldn't drive VBUS before seeing a sink's Rd, which R62/R63
  provide, but a non-compliant charger or a USB-A-to-C cable would see the board's
  5 V back-fed. **TODO (bench):** none of this was tried.

### The charger (U4, CS8501)

A single-cell charge + boost chip of the power-bank kind: VCC from `VBUS` through
R30‖R44 (0.28 Ω, a current-sense or soft-start element — inferred), BAT to the
cell, and a synchronous boost (LX via L2 4.7 µH, output through D12) onto `5V`. The
schematic's own PROG table gives **580 mA** for R64 = 1.2 kΩ. No datasheet was
found, so its termination voltage, its battery cut-off, and whether its boost
switches off at light load are unknown. Light-load shut-off is common in
power-bank chips and matters here: a sleeping ESP32 might draw too little to keep
the boost running. Measure it before relying on deep sleep on battery.

### Battery voltage reading (§1.2, §1.6)

VBAT → R18 20 kΩ → `EXIO_ADC` → R19 10 kΩ → GND, so V_adc = VBAT / 3. U10's ADC is
treated as 10-bit full scale at 3.3 V (demo), so

    VBAT = raw × 3 × 3.3 / 1023 = raw × 9.68 mV

The divider loads the cell with VBAT / 30 kΩ ≈ 0.14 mA at 4.2 V, always. Accuracy
depends on U10's ADC reference, which is presumably its VDD (the 3.3 V LDO output)
— so the reading is only as good as the LDO's tolerance (inferred). 1 % divider
resistors add little next to that.

With no cell fitted, 20 consecutive readings spread from 427 to 493 counts (4.13 V to
4.77 V; bench, 2026-10-26). An empty CC/CV charger output oscillates as it detects
"battery" and terminates repeatedly. That is the signature to show users as
"no battery".

### Backlight (§1.8)

U3 (AP3032) is a constant-current boost for the LED string. The current is set by
the FB voltage across R25‖R26 (5.1 Ω ∥ 5.1 Ω = 2.55 Ω). EN is `BL_EN_1`, driven
directly by U10 PD2 (EXIO2).

Dimming works by **injecting a filtered PWM into FB**: EXIO_PWM → R49 0 Ω → R50 10 kΩ
→ C34 100 nF (τ = 1 ms) → R51 68 kΩ → FB, with R48 (5.1 kΩ) from the sense node.
A higher PWM duty raises the average injected voltage. The regulator holds FB at its
reference, so it cuts LED current. That's why the PWM is inverted. The other option
on the sheet, PWM straight onto EN (R52), would have made duty proportional to
brightness. The bench showed brightness falling steadily from duty 0 to 230 and
going dark at 255, which is the FB-injection behavior. So that path is fitted and
R52 isn't (bench, 2026-10-26).

`setBacklightBrightness()` maps 100 % → duty 0 and 0 % → duty 247 (97 %, the
ceiling Waveshare's demo uses "to keep the screen from turning off completely").
Fully off is EN low: `lcdBacklight(false)`.

**TODO (datasheet):** with AP3032's FB reference (≈ 0.2 V for this class of
part), full-brightness current would be ≈ 0.2 / 2.55 ≈ 78 mA (inferred, unverified).

### Panel bias (§1.9)

U2 (AP3012) boosts 5 V to `AVDD_LCD` with R55 33 kΩ / R57 4.7 kΩ:
1.23 V × (1 + 33/4.7) ≈ 9.87 V, matching the sheet's "9.85V" label. Two charge
pumps from its switch node make VGH (≈ +16–18 V, zener-clamped) and VGL (negative,
6.8 V zener). VCOM is a trimmer divider from AVDD. All of them stop when EXIO6
pulls SHDN# low. R54 pulls SHDN# to 5 V, so the panel bias is **on before U10
runs** — and that pull-up is the possible overvoltage noted in errata item 13.

## RS-485: which pin is TX

The schematic's net labels and Waveshare's pin table put `RS485_TXD` on GPIO15. The
connectivity says otherwise (§1.13):

- GPIO15 is on U6 pin 1. U6 is an SP3485 in the standard 8-pin RS-485 pinout, where
  pin 1 is **RO**, the receiver output.
- GPIO16 is on U6 pin 4 (**DI**, driver input) and on U15's input, the buffer that
  feeds the auto-direction circuit. That circuit has to watch the transmit line to
  know when to enable the driver.
- Waveshare's own demo uses RX = 15, TX = 16.

The bench check (2026-10-26) read each pin with the ESP32's internal pull-up and
then its pull-down, with the bus idle. While TX idles high, Q1 holds RE#/DE low, so
the receiver is enabled and RO actively drives its pin.
- **GPIO16 followed the pulls (1 then 0): nothing drives it, so it cannot be RO.**
- GPIO15 read 1 under both. On its own that proves little, since R74's 4.7 kΩ
  pull-up would win against the internal pull-down anyway. The decisive half is
  GPIO16.

Verdict: **TX = GPIO16, RX = GPIO15.** The labels name the nets from the
transceiver's point of view.

Auto-direction (inferred from the circuit): U15 buffers TX into an RC (R76 200 kΩ
∥ D7, C51 1 nF) on Q1's gate. A low bit discharges the gate quickly through D7, Q1
turns off, R79 pulls RE#/DE high, and the driver drives A/B low. On a high bit the
gate recharges only through R76 (τ ≈ 200 µs), so the driver stays enabled, actively
driving the high level, for a while after each low bit. Once Q1 turns back on, the
fail-safe bias holds the idle level. The hold-over
limits how soon after a transmission the board can hear a reply, because RE# is
tied to DE.

Link test (bench, 2026-10-26), against an ESP32-P4-86-Panel's isolated THVD1406
port, 120 Ω at both ends. The 7B sent 64-byte packets; the P4 echoed each one.
- Every packet was clean from 9600 to **2.5 Mbaud**, including 2000 packets at
  2 Mbaud.
- 3 Mbaud failed (mostly lost packets), with replies delayed by anything from
  0 to 2 ms, so the limit is the signal path, not the turnaround.
- With no added delay, the echo starts as soon as the P4's UART driver delivers the
  64th byte. That is at least a couple of character times plus a task wake-up,
  about 100 µs or more at 115200. The 7B's receiver was back in time at every
  working speed, so Q1 turns back on within roughly a τ — sooner than the
  several-τ estimate (inferred).
- Which leg fails at 3 Mbaud wasn't isolated. **TODO (bench).**

## UART routing and uploads

UART0 (GPIO43 TX through R107 499 Ω, GPIO44 RX) passes through U13 (FSUSB42), steered
by SW1. In the UART1 position, SEL is low and UART0 reaches the CH343. In UART2,
SEL is pulled high by R17 and UART0 reaches H3 (§1.4). esptool gets *no bytes at
all* in the UART2 position, which first showed up on the bench as "Failed to
connect: No serial data received".

The CH343's DTR/RTS drive the usual crossed-transistor auto-reset (Q6, Q9) on EN and
GPIO0. GPIO0 is also panel G3, so it is driven during normal running but released
by the RGB peripheral at reset, which is when auto-reset needs it.

Flash reads at 460800 and 921600 baud failed repeatedly with "Invalid head of
packet" partway through; 230400 succeeded for all 16 MB with no retries. R107's
499 Ω in series with the TX line, plus U13's on-resistance and capacitance, would
round the edges at high baud rates — a plausible cause (inferred).
**TODO (bench):** scope GPIO43 at 921600, and try another host or driver.

## microSD (§1.12)

All four used card lines have 10 kΩ pull-ups (R88–R91), so SD mode's open-drain
CMD phase needs no internal pulls. D3 sits on EXIO4 with R88 pulling it up, so the
card stays in SD mode unless the expander drives D3 low. Waveshare's demo and our
bench run both used SD_MMC 1-bit mode on GPIO12/11/13.

## The -7's circuits

From the VER1.2 netlist; section numbers are
[schematic-description-7.md](schematic-description-7.md)'s.

### Battery divider (§1.2)

R18 200 kΩ / R19 100 kΩ, the same ÷3 as the -7B's 20 kΩ / 10 kΩ at a tenth of the
load: VBAT / 300 kΩ ≈ 14 µA at 4.2 V. The tap goes only to TP1. Wired to GPIO6
(ADC1_CH5), the source impedance is 200 k ∥ 100 k ≈ 67 kΩ — high for the ESP32's
SAR ADC, whose sampling capacitor wants a low source or a settling capacitor; a
100 nF from TP1 to GND would make single readings stable (inferred).

### Backlight (§1.8)

The same AP3032 boost with R25 = 1.8 Ω (R26 "1.6R/NC", taken as not fitted). At the
≈0.2 V FB reference assumed above, that is ≈ 0.2 / 1.8 ≈ 110 mA full-scale against the
-7B's ≈ 78 mA (inferred, unverified). The -7B's `EXIO_PWM` net is `BL_EN` here, and
R52 is a single-valued 0 Ω: `BL_EN` (pad P1) and the EN line `BL_EN_1` are one node,
driven by CH422G IO2 through R114 100 Ω. So an external PWM on P1 would contend
with IO2's push-pull output through 100 Ω; dimming needs IO2 left high and a PWM
source strong enough to override it, or R114 removed (inferred). The FB-injection
parts (R49–R51, C34) are drawn as NC options.

### Panel bias enable (§1.9)

R54 10 kΩ is a pull-down on `EXIO6` (SHDN# of the AP3012), not the -7B's pull-up to
5 V. That removes the -7B's question of 5 V on a 3.3 V expander pin, and makes the
bias default off. `board.begin()` drives IO6 high on both boards.

### I²C header level shifter (§1.11)

M1 (NDC7002N, two N-FETs) sits between `CTP_SDA/SCL` (3.3 V, R109/R110 pull-ups) and
the header's `D_SDA/D_SCL` (R108/R113 pull-ups to `I2C_VCC`). The symbol puts both
gates on `I2C_VCC`, the sources on the header side and the drains on the board side.
The classic BSS138 shifter is the other way round: gate on the *low* rail, source on
the low side. H2 is a 0 Ω link, fitted to 3V3 as shipped (bench, 2026-11-03); on 3V3 the orientation doesn't matter. With H2 on 5V:

- Gate 5 V, source (header) idle at 5 V: V_GS = 0, channel off.
- The body diode runs source → drain, i.e. from the 5 V header line into the 3.3 V
  bus, which R109/R110 can only hold down through 4.7 kΩ against it. The 3.3 V side
  would idle near 5 V − V_F, held lower only by the ESP32's, GT911's and CH422G's
  input clamps (inferred).

If the symbol's pin roles are right, the 5V position is unsafe. If Altium's S and D
are swapped relative to the package (the netlist can't tell), it is the textbook
circuit. **TODO (bench):** measure GPIO8 idle with H2 on 5V and nothing on the
header.

### RS-485 (§1.13)

DI is grounded; GPIO16 drives the NPN S1, whose collector holds DE/RE# with R76
(4.7 kΩ to 3V3). TX idle high → S1 on → DE low: receiving, driver off, bus at "1"
from R82 (A to 5V) / R73 (B to GND). TX low → S1 off → DE high → the driver drives
DI = 0. Each 0 bit is driven, each 1 bit is released bus, so the edge rate of 0→1
transitions is set by the bias and termination, not the driver (inferred). The
-7B's circuit (U15 buffer, Q1, RC hold) is the same idea with DI on TX.

U6 is on `5V`. The SP3485's recommended supply is 3.0–3.6 V (**TODO (datasheet)**:
absolute maximum), and RO swings to its supply, so GPIO15 sees 5 V through nothing
but the ESP32-S3's pad, with R74 pulling only toward 3.3 V. **Measured** (2026-11-03,
one VER1.2 unit, bus idle and unconnected): 5 V on U6.8 and U6.1, and 5 V at the
module's GPIO15. The ESP32-S3's pad clamp then carries whatever RO can source into `3V3`.

The only software lever is RE#: it is tied to DE, so holding **GPIO16 low** turns
S1 off, R76 lifts DE/RE# to 3.3 V (above the SP3485's V_IH even at 5 V supply), the
receiver tri-states RO, and R74 brings GPIO15 back to 3.3 V. The price is that the
driver is then on and drives DI's 0 onto the bus: about 1 mA into the bias
resistors with nothing attached (inferred), but a stuck state on a live network.
(bench, 2026-11-03): GPIO15 3.3 V with GPIO16 low, 5 V released. Hardware fixes: lift U6.8 to `3V3`, or a series
resistor (≈1–10 kΩ) between U6.1 and GPIO15.
