# Waveshare ESP32-S3-Touch-LCD-7 (V1.2) — schematic, as understood

A prose-and-tables reading of Waveshare's schematic for the **ESP32-S3-Touch-LCD-7**
(7" 800×480 RGB panel with capacitive touch, ESP32-S3-WROOM-1-N8R8 module, RS-485, CAN,
microSD, Li-ion charger). Its sibling for the -7B is
[schematic-description-7B.md](schematic-description-7B.md); the two boards are one
design with a few blocks changed, and this file describes the -7 in full where it
differs and points at the -7B file, section by section, where it doesn't.

**Source.** `ESP32-S3-Touch-LCD-7-Sch.pdf`, from Waveshare's repository for the board
(folder `Waveshare-ESP32-S3-Touch-LCD-7-main/hardware/schematics/`). An **Altium
Designer** export (PDF Creator "Altium Designer", Producer "llPDFLib 3.x", CreationDate
2025-11-06). Two pages: page 1 is the single sheet `ESP32-S3-Touch-LCD-7 V1.2.SchDoc`,
page 2 the PCB silkscreen, which reads **"ESP32-S3-Touch-LCD-7 / 800×480 Pixels /
VER1.2"**. No date or revision is printed in the title block. The silkscreen revision
of the bench unit (MAC d8:3b:da:a5:e5:c4) has **not** been compared with VER1.2 —
**TODO (bench)**. Older -7 revisions exist (the separate -7 library predating this one
treated IO6 as free, which V1.2 does not), so a board without "VER1.2" may differ.

**How it was read.** The **netlist was extracted from the PDF's metadata** — the
Altium bookmark outline, dumped with `pypdf` — so connectivity is exact: 206
components, 160 nets, 634 pins, every pin on exactly one net. It was then **diffed
against the -7B's netlist** extracted the same way. 191 reference designators are
common to both; among them, pins were compared by net name where the net is named and
by the set of shared-part pins where Altium named it automatically. **57 pins differ**,
and 15 parts exist only on the -7 (14 only on the -7B). Everything else is identical
in connectivity, which is what lets later sections say "as the -7B".

**Values** are not in the metadata. They were paired with reference designators
automatically, by position, from the page's text layer, for both boards with the same
script: for the 191 common parts, **175 pair to the same value on both sheets**. The 16
that didn't, and every -7-only part, were then read by eye from 420 dpi renders. A value
in a "same as the -7B" row is therefore the -7B's reading confirmed by an automatic
match, not a second reading by eye; the pairing is heuristic, so a misattribution is
possible though the connections are not. Net-label aliases (`A` / `B`) are carried over
from the -7B file where the -7 net has the same name; the five nets that exist only on
the -7 show only their own name.

**Labels**, as in the -7B file: **TODO (schematic)** for what the sheet doesn't settle,
**TODO (bench)** for what only a test settles, **TODO (datasheet)** for a part's
datasheet, **inferred** for a conclusion drawn from part numbers, values or the drawing.
Facts from elsewhere are tagged **(SDK)** — Waveshare's -7 repository, chiefly
`ESP32_IO_Expander/src/port/esp_io_expander_ch422g.c` and the `06_TWAItransmit`
example — **(silkscreen)** for page 2, and **(bench)** for a physical unit, dated.
`Ref.pin` is a component pin; `NET` in code font is a net name as printed.

The board library's topic files and `README-engineering.md` draw on this file. Like
its sibling it tries to be **complete rather than useful**; why a circuit is built the
way it is belongs in `README-engineering.md`.

---

## 0. What differs from the -7B

| Block | -7 (this file) | -7B | § |
|---|---|---|---|
| Module U8 | ESP32-S3-WROOM-1-**N8R8** (8 MB flash) | N16R8 (16 MB) | 1.1 |
| UART USB-C ground | Type_C1 ground and shell on `GND`; VBUS straight onto `VUART` | separate `GND1`, choke option L3, links R28/R32 | 1.2 |
| 5 V input terminal | none | J9 | 1.2 |
| Battery divider R18/R19 | **200 kΩ / 100 kΩ** 1 %, to test pad **TP1 only** | 20 kΩ / 10 kΩ, into the expander's ADC | 1.2 |
| I/O expander U10 | **CH422G**, each output through a 100 Ω series resistor | 20-pin MCU (CH32V003-class) | 1.6 |
| Panel clock R27 / C35 | 100 Ω, NC | 150 Ω, 8.2 pF | 1.7 |
| Backlight current sense R25 / R26 | **1.8 Ω**, 1.6 Ω/NC | 5.1 Ω ‖ 5.1 Ω | 1.8 |
| Backlight dimming | **none from the ESP32**: EN only, plus pad P1 | expander PWM into FB | 1.8 |
| Panel-bias enable pull (R54) | 10 kΩ pull-**down** on `EXIO6`: bias **off** until driven | pull-up to 5 V: on by default | 1.9 |
| I²C header H1 | own rail `I2C_VCC` (0 Ω link position H2, 3V3 or 5V) through level shifter M1 | straight onto the touch bus at 3V3 | 1.11 |
| RS-485 | U6 on **5V**; DI grounded; direction by NPN **S1** from GPIO16 | U6 on 3.3 V; DI on GPIO16; U15/Q1 auto-direction | 1.13 |
| RS-485 TVS D16 | SMAJ12CA | SMAJ6.5CA | 1.13 |
| Termination | two 3-pin jumpers, **J4** (RS-485) and **J5** (CAN), "120R / NC" | DIP switch SW2 | 1.15 |
| GPIO6 header J8 | net `AD`, silkscreen "Sensor AD" | net `GPIO6` | 1.16 |

Everything not in this table — the USB-UART and its selector SW1, the native-USB / CAN
switch U9, the charger, the regulator, the panel connector and its straps, the bias
generator, touch, microSD, CAN — has the same parts and connections as the -7B.

---

## 1. Bill of materials, by function

### 1.1 Processor and module

As [-7B §1.1](schematic-description-7B.md#11-processor-and-module), with U8 an
**ESP32-S3-WROOM-1-N8R8**. (bench, 2026-11-03, MAC d8:3b:da:a5:e5:c4): 8 MB flash,
8 MB PSRAM. Strapping resistors R95/R103, R96/R104, R97/R105 and the RESET / BOOT
buttons K1 / K2 are as the -7B.

### 1.2 Power path

As [-7B §1.2](schematic-description-7B.md#12-power-path), except:

| Ref | Part | Role |
|---|---|---|
| Type_C1 | USB-C, silkscreen **UART1** | VBUS pins (A4/B9, A9/B4) **directly on `VUART`**; GND pins and shell (A1/B12, A12/B1, MTB) on **`GND`** — there is no `GND1` net and no `VUART'` |
| TVS1 | LTVS16H5.0ET5G (as -7B) | `VUART` → GND |
| R18, R19 | **200 kΩ 1 %, 100 kΩ 1 %** | `VBAT` → `NetR18_1` → GND: VBAT ÷ 3 to **TP1** (test pad) and nothing else |
| TP1 | test pad | the divider tap |

Not fitted on the -7 (no footprint on the sheet): L3, R28, R32, J9. The charger U4
(CS8501), its PROG table (R64 1.2 kΩ, 580 mA), Q5/Q10 ideal diodes, D11, D12, L2 and
the charge LEDs are as the -7B. Inferred from the connectivity, as on the -7B: only
Type_C1 charges the battery; Type_C2's VBUS pins carry the board's `5V` whenever it is
powered; there is no power switch.

The divider draws VBAT / 300 kΩ, about **14 µA** at 4.2 V, always (the -7B's draws
about 140 µA). Nothing on the board can read it: the CH422G has no ADC, and TP1 reaches
no GPIO.

### 1.3 Regulators

As [-7B §1.3](schematic-description-7B.md#13-regulators): U5 SGM2212-3.3 from `5V`, no
enable; R20 `3V3` → `LCD_VDD`.

### 1.4 USB-UART, auto-reset and the UART selector

As [-7B §1.4](schematic-description-7B.md#14-usb-uart-auto-reset-and-the-uart-selector):
CH343P U1, auto-reset Q6/Q9, FSUSB42 U13, slide switch **SW1** (silkscreen **UART1 /
UART2**) and header H3 (silkscreen **UART2 3V3 GND RXD TXD**). (bench, 2026-11-03):
the bench -7 enumerates as `wchusbserial59591146551` and uploads at 230400 and 460800
with SW1 on UART1.

### 1.5 Native USB / CAN selector

As [-7B §1.5](schematic-description-7B.md#15-native-usb--can-selector): U9 FSUSB42UMX,
SEL on `EXIO5` / `USB_SEL`, R117 10 kΩ pull-down. `USB_SEL` high routes GPIO20 → CAN
TXD and GPIO19 ← CAN RXD; the SDK's CAN example drives it high (SDK). (bench,
2026-11-03): `selectCan(true)` and TWAI at 500 kbit/s on GPIO20/19 exchanged frames
with a -7B with no loss.

### 1.6 I/O expander (U10)

| Ref | Part | Role |
|---|---|---|
| U10 | **CH422G** (printed) | I²C I/O expander. Not register-addressed: each function is its own I²C address (SDK): 0x24 system, 0x38 IO0–7 write, 0x26 IO0–7 read, 0x23 OC0–3 write. (bench, 2026-11-03): an address-only scan finds it answering **every address 0x20–0x27 and 0x30–0x3F**. |
| C47, C48 | 1 µF, 100 nF | U10 VCC |
| R111, R115, R16, R114, R116 | **100 Ω** each | series resistors, U10 → `EXIO4`, `EXIO5`, `EXIO6`, `BL_EN_1`, `CTP_RST` |
| R112 | **NC/100R** | series resistor, U10 IO3 → `EXIO3` / `LCD_RST`. Variant: if NC, the panel reset is held only by R11's pull-up and IO3 does nothing. **TODO (bench):** which is fitted. |

| U10 pin | Name | Net | Function |
|---|---|---|---|
| 1 | IO4 | → R111 → `EXIO4` / `SDCS` | microSD CS (via R92) |
| 2 | IO5 | → R115 → `EXIO5` / `USB_SEL` | U9 SEL |
| 3 | IO6 | → R16 → `EXIO6` / `LCD_VDD_EN` | U2 SHDN# (panel bias), R54 pull-down |
| 4 | IO7 | — | unconnected |
| 5 | SCL | `CTP_SCL` / `SCL` | I²C SCL (GPIO9) |
| 6 | SDA | `CTP_SDA` / `SDA` | I²C SDA (GPIO8) |
| 7 | GND | GND | |
| 8–11 | OC0–OC3 | — | unconnected |
| 12 | IO0 | — | unconnected |
| 13 | IO1 | → R116 → `CTP_RST` / `EXIO1` | touch reset (J2.1) |
| 14 | VCC | `3V3` | |
| 15 | IO2 | → R114 → `BL_EN_1` / `DISP` / `EXIO2` | backlight boost EN (U3.4) |
| 16 | IO3 | → R112 → `EXIO3` / `LCD_RST` | panel reset (J1.44), R11 pull-up |

The EXIO numbering is the same as the -7B's, so drivers address both boards' lines by
the same numbers. The CH422G's system register (SDK, `esp_io_expander_ch422g.c`): bit 0
**IO_OE** (IO0–7 outputs), bit 1 **A_SCAN**, bit 2 **OD_EN**, bit 3 **SLEEP**; power-on
value 0x01. IO0–7 switch between input and output together (IO_OE), not per pin.

### 1.7 Display (J1)

As [-7B §1.7](schematic-description-7B.md#17-display-j1) — the same 50-pin connector and
pin-out, the same straps (R1, R6–R9, R11–R13) — with the panel clock filter changed:

| Ref | Part | Role |
|---|---|---|
| R27, C35 | **100 Ω** series, **NC** | `IO7'` (GPIO7) → `PCLK` at J1.37 |

Silkscreen "800×480 Pixels" (silkscreen). The panel is an 800×480 ST7262 (from the
separate -7 library and Waveshare's demo, not the sheet); its timing, 820 × 500 total at
14–16 MHz, is in the display library.

### 1.8 Backlight

| Ref | Part | Role |
|---|---|---|
| U3 | AP3032 (as -7B) | EN `BL_EN_1`, FB `NetR48_2` |
| L6, D6, C25–C27 | as -7B | |
| R25, R26 | **1.8 Ω**, **1.6R/NC** | `VLED-` → GND: LED current sense |
| R48 | 5.1K/0R/NC (as -7B) | `VLED-` → FB |
| R21, R22 | 4.7K/NC, 100K/NC (as -7B) | `BL_EN_1` pull-up / pull-down options |
| R52 | **0R** (single value) | `BL_EN` → `BL_EN_1` |
| P1 | test pad | `BL_EN` |
| R49, R50, C34, R51 | 0R/NC, 10k/NC, 100NF/NC, 68K/NC (as -7B) | `BL_EN` → RC → FB, the PWM-into-FB option |

On the -7 the net the -7B calls `EXIO_PWM` is `BL_EN`, and it reaches **no ESP32 or
expander pin** — only pad P1 and, through R52 0 Ω, the boost's EN line `BL_EN_1`,
which the CH422G drives through R114. So the backlight is **on/off from IO2**, and the
only dimming path is an external PWM on P1, which would fight IO2's push-pull output
through R114's 100 Ω (inferred). With R26 not fitted, the sense resistor is 1.8 Ω
against the -7B's 2.55 Ω, so the -7's LED current is about 1.4 × the -7B's at the same
FB reference (inferred; the AP3032's FB voltage is **TODO (datasheet)**).

### 1.9 Panel bias ("Panel_Voltage")

As [-7B §1.9](schematic-description-7B.md#19-panel-bias-panel_voltage) — AP3012 U2, the
9.85 V `AVDD_LCD`, the VGH / VGL charge pumps and the VCOM divider — except:

| Ref | Part | Role |
|---|---|---|
| R54 | 10 kΩ | `EXIO6` / `LCD_VDD_EN` → **GND**: bias **off until IO6 drives it high** |
| R16 | 100 Ω | U10 IO6 → `EXIO6` |

The -7B's R54 is a pull-up from `EXIO6` to U2's 5 V input, which puts 5 V on an
expander pin through 10 kΩ (its errata item 13). The -7's is a pull-down, so that
hazard is absent here; the cost is that the panel's bias rails stay off until the
expander is configured.

### 1.10 Touch (J2)

As [-7B §1.10](schematic-description-7B.md#110-touch-j2): GT911 on the panel flex, INT
on GPIO4, reset on `CTP_RST` (here through R116). (bench, 2026-11-03): GT911 at 0x5D
after reset, configuration registers 0x8048–0x804B read **800 × 480**, product ID "911".

### 1.11 The I²C bus

`CTP_SDA` / `SDA` = GPIO8, `CTP_SCL` / `SCL` = GPIO9, R109 / R110 4.7 kΩ to `3V3`, as the
-7B. The header is isolated from it by a level shifter:

| Ref | Part | Role |
|---|---|---|
| H1 | 4-pin header, silkscreen **I2C VCC GND SDA SCL** | 1 `I2C_VCC`, 2 GND, 3 `D_SDA`, 4 `D_SCL` |
| H2 | 3-pad **0 Ω resistor position**, silkscreen **I2C 3V3 5V**; sheet note "默认贴3V3" ("fitted to 3V3 by default") | 1 `3V3`, 2 `I2C_VCC`, 3 `5V`. (bench, 2026-11-03): the resistor is on the 3V3 side. Moving it to 5V is a solder job. |
| M1 | **NDC7002N** dual N-MOSFET | 1 G1 `I2C_VCC`; 2 S2 `D_SCL`; 3 G2 `I2C_VCC`; 4 D2 `CTP_SCL`; 5 S1 `D_SDA`; 6 D1 `CTP_SDA` (pin roles from the symbol) |
| R108, R113 | 4.7 kΩ | `D_SDA`, `D_SCL` pull-ups to `I2C_VCC` |

| Device | Where | 7-bit address |
|---|---|---|
| I/O expander | U10 | 0x20–0x27 and 0x30–0x3F all answer (bench, 2026-11-03); functions at 0x23, 0x24, 0x26, 0x38 (SDK) |
| GT911 touch | J2 | 0x5D / 0x14 |
| header H1 | through M1 | |

With H2 on 3V3 both sides of M1 are at 3.3 V and it is transparent. **With H2 on 5V**,
M1 as drawn has its gates on the 5 V rail and its sources on the header side; each
FET's body diode then points from the 5 V header side into the board's 3.3 V bus,
which could lift `CTP_SDA` / `CTP_SCL` above 3.3 V when idle (inferred from the symbol's
pin roles). The usual shifter has the gates on the low-voltage rail and the sources on
the low-voltage side. **TODO (bench):** with H2 on 5V and the bus idle, measure GPIO8 /
GPIO9; until then, treat the 5V position as suspect.

### 1.12 TF / microSD (SD1)

As [-7B §1.12](schematic-description-7B.md#112-tf--microsd-sd1): SPI or 1-bit SD on
GPIO11 / 12 / 13, CS on `EXIO4` (here through R111 and R92), D1/D2 unwired, no card
detect.

### 1.13 RS-485

| Ref | Part | Role |
|---|---|---|
| U6 | **SP3485EN** | 1 RO `IO15` / `RS485_TXD` → **GPIO15**; 2 RE#, 3 DE both `NetR76_2`; 4 DI **GND**; 5 GND; 6 A `A1`; 7 B `B1`; 8 VCC **`5V`** |
| C16 | 100 nF | U6 VCC (`5V`) |
| R74 | 4.7 kΩ | GPIO15 / RO pull-up to `3V3` |
| R76 | 4.7 kΩ | RE#/DE pull-up to `3V3` |
| S1 | NPN transistor, **part number not printed** | 1 B `NetR81_2`; 2 E GND; 3 C `NetR76_2` (RE#/DE). **TODO (schematic):** part. |
| R81 | 4.7 kΩ | **GPIO16** (`IO16` / `RS485_RXD`) → S1 base |
| R78 | 4.7 kΩ | GPIO16 pull-up to `3V3` |
| R82, R73 | 10 kΩ, 10 kΩ | A pull-up to **`5V`**; B pull-down to GND (fail-safe bias) |
| C29, C28 | NC, NC | options across the bias resistors |
| D15, D16, D17 | **SMAJ12CA** ×3 | B–`EARTH`, A–B, A–`EARTH` |
| R86 | 0 Ω | `EARTH` → GND |
| F1, F2 | fuse, value not printed | `B1` → `RS485_TX-`, `A1` → `RS485_TX+`. **TODO (schematic):** rating. |
| J7 | 2-pin terminal, silkscreen **RS-485 A B** | as -7B |
| R70 | 120 Ω | `B1` → J4.3 (termination, §1.15) |

**Direction: GPIO16 transmits, GPIO15 receives**, as on the -7B and with the same
swapped net labels. The transmit path is different: DI is tied to GND, and GPIO16
drives DE/RE# through S1. GPIO16 high (idle) turns S1 on, holding DE/RE# low: receiver
on, driver off, the bus idling at "1" from R82 / R73. GPIO16 low turns S1 off, R76
pulls DE high, and the driver drives DI's 0 onto the bus. So a UART on GPIO16 sends
its 0 bits actively and its 1 bits as released bus (inferred). Waveshare's -7 RS-485
example should agree — **TODO (SDK)**: not checked.

**U6 runs from `5V`.** The SP3485 is specified for a 3.3 V supply (**TODO
(datasheet)**: its absolute maximum), and RO drives GPIO15 directly, with R74 only a
pull-up, so in receive GPIO15 is driven to 5 V, above the ESP32-S3's 3.3 V I/O
rating. (bench, 2026-11-03, one VER1.2 unit, bus terminals unconnected): **5 V on
U6.8 (VCC) and U6.1 (RO), and 5 V at the module's GPIO15.** RE# and DE are tied, so with GPIO16 low (S1
off, RE# high) the receiver is disabled and RO goes high-impedance, releasing GPIO15
to R74 — at the cost of the driver driving the bus low. (bench, 2026-11-03): with
GPIO16 held low GPIO15 read **3.3 V**; released, it returned to 5 V.

### 1.14 CAN

As [-7B §1.14](schematic-description-7B.md#114-can): TJA1051T/3 U7 at 5 V with VIO 3V3,
normal mode (R77 0 Ω), SM24CANB D18, terminal J6, R75 120 Ω to the termination jumper.

### 1.15 Termination jumpers

| Ref | Part | Role |
|---|---|---|
| J4 | 3-pin header with a removable jumper (bench, 2026-11-03), silkscreen **RS-485 120R NC** | 1 (A) unconnected; 2 (COM) `A1`; 3 (B) → R70 120 Ω → `B1`. COM–B puts 120 Ω across A–B. |
| J5 | 3-pin header with a removable jumper (bench, 2026-11-03), silkscreen **CAN 120R NC** | 1 (A) unconnected; 2 (COM) → R75 120 Ω → `CANH`; 3 (B) `CANL`. COM–B puts 120 Ω across CANH–CANL. |

### 1.16 LED, headers, test points

| Ref | Part | Role |
|---|---|---|
| PWR1, R106 | red LED, 1 kΩ (as -7B) | on with `3V3` |
| J8 | 3-pin header, silkscreen **Sensor AD: 3V3 GND AD** | 1 `3V3`, 2 GND, 3 `AD` (**GPIO6**, U8.6) |
| H1, H2, H3, J4–J7 | | §1.11, §1.4, §1.15, §1.14, §1.13 |
| TP1 | test pad | battery divider (§1.2) |
| P1 | test pad | `BL_EN` (§1.8) |

---

## 2. The complete GPIO assignment

The module's pins carry the same nets as on the -7B — the netlist diff found no
differing U8 pin except the name of GPIO6's net — so the table in
[-7B §2](schematic-description-7B.md#2-the-complete-gpio-assignment) applies, with these
rows changed:

| GPIO | Module pin | Net | Use | Notes |
|---|---|---|---|---|
| 6 | 6 | `AD` | **J8 pin 3** ("Sensor AD") | **free**; ADC1_CH5 |
| 8, 9 | 12, 17 | `CTP_SDA`, `CTP_SCL` | I²C | U10, GT911, and H1 **through M1** (§1.11) |
| 15 | 8 | `IO15` | RS-485 **RX** (U6 RO) | R74 4.7 kΩ to `3V3`; **U6 on 5 V drives it** (§1.13) |
| 16 | 9 | `IO16` | RS-485 **TX** (S1 base via R81) | R78 4.7 kΩ pull-up |
| 35, 36, 37 | 28, 29, 30 | — | unconnected | used by the N8R8's octal PSRAM |

As on the -7B, GPIO6 is the only GPIO with nothing else on it; GPIO43/44 (H3, SW1 on
UART2) and GPIO19/20 (U9) are the only others a sketch can have without giving up a
function. The sheet's own GPIO table (bottom right) has not been re-checked row by row
against the netlist — **TODO (schematic)**.

### I/O expander lines

| EXIO | U10 pin | Net | Use |
|---|---|---|---|
| IO0 | 12 | — | unconnected |
| EXIO1 | 13 (IO1) | `CTP_RST` via R116 | GT911 reset |
| EXIO2 | 15 (IO2) | `BL_EN_1` via R114 | AP3032 EN (backlight on/off) |
| EXIO3 | 16 (IO3) | `LCD_RST` via R112 (NC/100R) | panel reset |
| EXIO4 | 1 (IO4) | `SDCS` via R111 | microSD CS |
| EXIO5 | 2 (IO5) | `USB_SEL` via R115 | U9: 0 = USB, 1 = CAN |
| EXIO6 | 3 (IO6) | `LCD_VDD_EN` via R16 | AP3012 SHDN# (panel bias), R54 pull-down |
| IO7 | 4 | — | unconnected |
| OC0–OC3 | 8–11 | — | unconnected |

## 3. Power rails

As [-7B §3](schematic-description-7B.md#3-power-rails), except:

| Rail | Source | Switchable? | Feeds |
|---|---|---|---|
| `VUART` | Type_C1 VBUS, directly | — | TVS1; Q5 → `VBUS`; Q10 → `5V` |
| `5V` | Type_C2 VBUS; Q10; D11; U4 VOUT / D12 (no J9) | no | as -7B, plus **U6 (RS-485)** and its A-line bias R82 |
| `VBAT` | J3; U4 | no | U4; R18/R19 divider (≈14 µA) to TP1 |
| `AVDD_LCD`, `VGH`, `VGL`, `VCOM` | U2 | **EXIO6**, default **off** (R54 pull-down) | as -7B |
| `VLED+` | U3 | **EXIO2** only; no dimming input | panel backlight |
| `I2C_VCC` | `3V3` or `5V` by the 0 Ω link at H2 (3V3 as shipped) | by rework only | H1.1, R108, R113, M1 gates |

There is no `RS485_VCC`, `GND1` or `VUART'` on the -7.

## 4. What the sheet does not say

- **Whether the bench unit is VER1.2** — not checked. **TODO (bench).**
- **R112's fitted option** (NC or 100 Ω): decides whether IO3 can reset the panel.
  **TODO (bench).**
- **S1's part**, **F1/F2's rating** — not printed. **TODO (schematic).**
- **SP3485 at 5 V**: (bench, 2026-11-03) GPIO15 sees 5 V. Whether the SP3485
  itself is within its absolute maximum at 5 V — **TODO (datasheet).**
- **M1 with H2 on 5V**: whether the body diodes lift the 3.3 V bus, as the pin roles
  suggest. **TODO (bench).**
- **Backlight current**: R25 alone sets it; the AP3032's FB reference is **TODO
  (datasheet)**.
- **The panel**: glass and controller are not on the sheet; "800×480 Pixels"
  (silkscreen), ST7262 (demo).
- The -7B file's open items for the shared blocks — the CS8501's behaviour, the CH343's
  V3 pin, the VCOM note, the variant parts in the backlight block — apply here too.
- **Current draw** in any state — not measured. **TODO (bench).**

---

## Appendix A. Named nets

Pins are `Ref-pin` as the Altium outline gives them; `0` is an exposed pad or mounting
tab. Aliases are carried over from the -7B file for nets of the same name (see the
opening paragraphs); `AD`, `BL_EN`, `D_SCL`, `D_SDA` and `I2C_VCC` exist only here.

| Net (aliases) | Pins |
|---|---|
| `3V3` | C10-2 C11-2 C12-2 C23-2 C24-2 C30-2 C40-2 C42-2 C45-2 C46-2 C47-1 C48-1 H2-1 H3-1 J2-2 J8-1 R17-1 R20-2 R74-1 R76-1 R78-2 R88-2 R89-2 R90-2 R91-1 R93-2 R95-1 R96-1 R97-1 R100-2 R106-1 R109-1 R110-1 SD1-4 U1-1 U1-6 U5-2 U7-5 U8-2 U9-9 U10-14 U13-9 |
| `5V` | C3-2 C4-2 C8-2 C9-2 C15-2 C16-2 C26-2 C29-1 D11-2 D12-2 H2-3 L6-1 Q10-2 Q12-2 R21-1 R43-1 R82-1 TVS2-1 Type_C2-A4/B9 Type_C2-A9/B4 U3-6 U4-1 U5-3 U6-8 U7-3 |
| `A1` | C29-2 D16-2 D17-1 F2-2 J4-2 R82-2 U6-6 |
| `AD` | J8-3 U8-6 |
| `AVDD_LCD` | C31-1 C32-2 C33-2 D3-1 D5-2 J1-43 R45-2 R55-2 |
| `B1` | C28-1 D15-2 D16-1 F1-2 R70-1 R73-1 U6-7 |
| `B3` / `IO14` | J1-16 U8-22 |
| `B4` / `IO38` | J1-15 U8-31 |
| `B5` / `IO18` | J1-14 U8-11 |
| `B6` / `IO17` | J1-13 U8-10 |
| `B7` / `IO10` | J1-12 U8-18 |
| `BL_EN` | P1-1 R49-1 R52-1 |
| `BL_EN_1` / `DISP` / `EXIO2` | R21-2 R22-1 R52-2 R114-1 U3-4 |
| `CANH` | D18-1 J6-2 R75-1 U7-7 |
| `CANL` | D18-2 J5-3 J6-1 U7-6 |
| `CANRX` | U7-4 U9-6 |
| `CANTX` | U7-1 U9-7 |
| `CC1` | R62-2 Type_C2-A5 |
| `CC2` | R63-2 Type_C2-B5 |
| `CH_RXD` | U1-4 U13-5 |
| `CH_TXD` | U1-5 U13-4 |
| `CLK` / `DCLK` / `IO7` / `PCLK` | C35-2 J1-37 R27-1 |
| `CTP_IRQ` / `IO4` | J2-4 U8-4 |
| `CTP_RST` / `EXIO1` | J2-1 R116-1 |
| `CTP_SCL` / `IO9` / `SCL` | J2-6 M1-4 R110-2 U8-17 U10-5 |
| `CTP_SDA` / `IO8` / `SDA` | J2-5 M1-6 R109-2 U8-12 U10-6 |
| `D_SCL` | H1-4 M1-2 R113-2 |
| `D_SDA` | H1-3 M1-5 R108-2 |
| `DCC1` | R2-2 Type_C1-A5 |
| `DCC2` | R3-2 Type_C1-B5 |
| `DE` / `IO5` | J1-9 U8-5 |
| `DTR` | Q9-2 R4-2 U1-12 |
| `EARTH` | D15-1 D17-2 R86-2 |
| `ESP_RXD` / `RX0` | U8-36 U13-1 |
| `ESP_TXD` | R107-1 U13-2 |
| `ESP_USB_N` | U8-13 U9-2 |
| `ESP_USB_P` | U8-14 U9-1 |
| `ESP_USBH_N` | R65-1 U9-4 |
| `ESP_USBH_P` | R66-1 U9-5 |
| `EX_RXD` | H3-3 U13-7 |
| `EX_TXD` | H3-4 U13-6 |
| `EXIO3` / `LCD_RST` | J1-44 R11-1 R112-1 |
| `EXIO4` / `SDCS` | R92-2 R111-2 |
| `EXIO5` / `USB_SEL` | R115-2 R117-1 U9-10 |
| `EXIO6` / `LCD_VDD_EN` | R16-2 R54-1 U2-4 |
| `G2` / `IO39` | J1-25 U8-32 |
| `G3` / `IO0` | J1-24 K2-1 Q9-3 R100-1 U8-27 |
| `G4` / `IO45` | J1-23 R96-2 R104-1 U8-26 |
| `G5` / `IO48` | J1-22 U8-25 |
| `G6` / `IO47` | J1-21 U8-24 |
| `G7` / `IO21` | J1-20 U8-23 |
| `GND` | C1-1 C2-2 C3-1 C4-1 C5-1 C6-1 C7-1 C8-1 C9-1 C10-1 C11-1 C13-2 C14-2 C16-1 C17-2 C20-2 C21-2 C22-2 C23-1 C24-1 C25-1 C26-1 C27-1 C28-2 C30-1 C31-2 C32-1 C33-1 C34-1 C35-1 C36-2 C37-2 C38-2 C39-2 C40-1 C42-1 C43-1 C45-1 C46-1 C47-2 C48-2 D1-2 D2-2 D4-1 D8-2 D9-2 D13-2 D14-2 H1-2 H3-2 J1-5 J1-17 J1-18 J1-19 J1-26 J1-27 J1-33 J1-34 J1-35 J1-36 J1-38 J1-48 J1-51 J1-52 J2-0 J2-3 J3-0 J3-1 J8-2 K1-1 K2-2 PWR1-K R2-1 R3-1 R6-2 R9-2 R12-2 R15-2 R19-1 R22-2 R23-2 R24-2 R25-1 R26-1 R42-2 R54-2 R57-1 R59-1 R62-1 R63-1 R64-1 R67-2 R68-2 R73-2 R86-1 R87-1 R103-2 R104-2 R105-2 R117-2 S1-2 SD1-6 SD1-9 SD1-10 SW1-1 SW1-4 TVS1-2 TVS2-2 Type_C1-A1/B12 Type_C1-A12/B1 Type_C1-MTB Type_C2-A1/B12 Type_C2-A12/B1 Type_C2-MTB U1-0 U1-2 U2-2 U3-2 U4-0 U4-7 U5-1 U6-4 U6-5 U8-1 U8-40 U8-41 U9-3 U9-8 U10-7 U13-3 U13-8 |
| `HS` / `HSYNC` / `IO46` | J1-11 R97-2 R105-1 U8-16 |
| `I2C_VCC` | H1-1 H2-2 M1-1 M1-3 R108-1 R113-1 |
| `IO1` / `R3` | J1-32 U8-39 |
| `IO11` / `MOSI` | R98-2 U8-19 |
| `IO12` / `SCK` | R99-2 U8-20 |
| `IO13` / `MISO` | R101-2 U8-21 |
| `IO15` / `RS485_TXD` | R74-2 U6-1 U8-8 |
| `IO16` / `RS485_RXD` | R78-1 R81-1 U8-9 |
| `IO2` / `R4` | J1-31 U8-38 |
| `IO3` / `VS` / `VSYNC` | J1-10 R95-2 R103-1 U8-15 |
| `IO40` / `R7` | J1-28 U8-33 |
| `IO41` / `R6` | J1-29 U8-34 |
| `IO42` / `R5` | J1-30 U8-35 |
| `IO7'` | R27-2 U8-7 |
| `LCD_VDD` | J1-7 R1-2 R7-2 R8-2 R11-2 R13-2 R20-1 |
| `RESET` | C43-2 K1-2 Q6-3 R93-1 U8-3 |
| `RS485_TX+` | F2-1 J7-1 |
| `RS485_TX-` | F1-1 J7-2 |
| `RTS` | Q6-2 R29-2 U1-13 |
| `SGND` | C12-1 C15-1 D18-3 R77-1 R87-2 U7-2 |
| `TX0` | R107-2 U8-37 |
| `UART_H` | R17-2 SW1-3 SW1-6 |
| `UART_SEL` | SW1-2 SW1-5 U13-10 |
| `USB_N` | R5-1 U1-8 |
| `USB_P` | R10-1 U1-7 |
| `USBD_N` | D8-1 R5-2 Type_C1-A7 Type_C1-B7 |
| `USBD_P` | D9-1 R10-2 Type_C1-A6 Type_C1-B6 |
| `USBH_N` | D13-1 R65-2 Type_C2-A7 Type_C2-B7 |
| `USBH_P` | D14-1 R66-2 Type_C2-A6 Type_C2-B6 |
| `VBAT` | C6-2 C7-2 J3-2 L2-2 R18-2 U4-6 |
| `VBUS` | C1-2 C2-1 D11-1 Q5-2 Q8-2 R30-2 R44-2 U1-3 U1-9 |
| `VCOM` | C37-1 C38-1 J1-6 J1-46 R53-1 R58-2 |
| `VGH` | C21-1 C22-1 D4-2 J1-41 R41-1 R42-1 |
| `VGL` | C13-1 C17-1 D1-1 J1-42 R14-1 R15-1 |
| `VLED+` | C25-2 C27-2 D6-2 J1-1 J1-2 U3-5 |
| `VLED-` | J1-3 J1-4 R25-2 R26-2 R48-1 |
| `VUART` | Q5-3 Q7-2 Q10-3 Q11-2 TVS1-1 Type_C1-A4/B9 Type_C1-A9/B4 |

## Appendix B. Unnamed nets

Altium's automatic names (`Net<Ref>_<pin>`). Descriptions are the -7B file's where the
net has the same name and the same pins on the shared parts; the rest were written for
this board. Single-pin entries are unconnected pins.

| Net | Pins | What it is |
|---|---|---|
| `NetC5_2` | C5-2 R30-1 R31-2 R44-1 U4-5 | U4 VCC |
| `NetC14_1` | C14-1 D2-1 R14-2 | VGL pump reservoir |
| `NetC18_1` | C18-1 C19-1 D5-1 L1-2 U2-1 | U2 switch node |
| `NetC18_2` | C18-2 D2-3 | VGL pump flying node |
| `NetC19_2` | C19-2 D3-3 | VGH pump flying node |
| `NetC20_1` | C20-1 D3-2 R41-2 | VGH pump reservoir |
| `NetC34_2` | C34-2 R50-2 R51-1 | backlight PWM filter node |
| `NetC36_1` | C36-1 C39-1 L1-1 R43-2 U2-5 | U2 VIN (from `5V` via R43); unlike the -7B, R54 is not on it |
| `NetD6_1` | D6-1 L6-2 U3-1 | U3 switch node |
| `NetD12_1` | D12-1 L2-1 U4-8 | U4 LX |
| `NetJ1_8` | J1-8 R1-1 | panel MODE strap |
| `NetJ1_39` | J1-39 R6-1 R7-1 | panel L/R strap |
| `NetJ1_40` | J1-40 R8-1 R9-1 | panel U/D strap |
| `NetJ1_45` | J1-45 | panel NC pins |
| `NetJ1_47` | J1-47 R12-1 R13-1 | panel DITHB strap |
| `NetJ1_49` | J1-49 | panel NC pins |
| `NetJ1_50` | J1-50 | panel NC pins |
| `NetJ4_1` | J4-1 | J4.1 ("NC" position) — unconnected |
| `NetJ4_3` | J4-3 R70-2 | J4.3 → R70 → `B1`: RS-485 termination |
| `NetJ5_1` | J5-1 | J5.1 ("NC" position) — unconnected |
| `NetJ5_2` | J5-2 R75-2 | J5.2 (COM) → R75 → `CANH`: CAN termination |
| `NetLed1_A` | Led1-A Led2-A R31-1 | charge-LED anodes |
| `NetLed1_K` | Led1-K U4-2 | CHRG# |
| `NetLed2_K` | Led2-K U4-4 | STDBY# |
| `NetPWR1_A` | PWR1-A R106-2 | power LED |
| `NetQ5_1` | Q5-1 Q8-3 R24-1 | Q5 gate |
| `NetQ6_1` | Q6-1 R4-1 | Q6 base |
| `NetQ7_1` | Q7-1 Q7-3 Q8-1 R23-1 | Q7/Q8 bases |
| `NetQ9_1` | Q9-1 R29-1 | Q9 base |
| `NetQ10_1` | Q10-1 Q12-3 R68-1 | Q10 gate |
| `NetQ11_1` | Q11-1 Q11-3 Q12-1 R67-1 | Q11/Q12 bases |
| `NetR16_1` | R16-1 U10-3 | U10 IO6 → R16 |
| `NetR18_1` | R18-1 R19-2 TP1-1 | VBAT ÷ 3, to TP1 only |
| `NetR45_1` | R45-1 R53-2 R56-2 R58-1 | VCOM divider top |
| `NetR48_2` | R48-2 R51-2 U3-3 | U3 FB |
| `NetR49_2` | R49-2 R50-1 | backlight PWM filter |
| `NetR55_1` | R55-1 R57-2 U2-3 | U2 FB |
| `NetR56_1` | R56-1 R58-3 R59-2 | VCOM divider bottom |
| `NetR64_2` | R64-2 U4-3 | U4 PROG |
| `NetR76_2` | R76-2 S1-3 U6-2 U6-3 | U6 RE#/DE; R76 pull-up; S1 collector |
| `NetR77_2` | R77-2 U7-8 | U7 S |
| `NetR81_2` | R81-2 S1-1 | S1 base ← R81 ← GPIO16 |
| `NetR88_1` | R88-1 R92-1 SD1-2 | card CD/D3 (CS) |
| `NetR89_1` | R89-1 R98-1 SD1-3 | card CMD |
| `NetR90_1` | R90-1 R99-1 SD1-5 | card CLK |
| `NetR91_2` | R91-2 R101-1 SD1-7 | card D0 |
| `NetR111_1` | R111-1 U10-1 | U10 IO4 → R111 |
| `NetR112_2` | R112-2 U10-16 | U10 IO3 → R112 |
| `NetR114_2` | R114-2 U10-15 | U10 IO2 → R114 |
| `NetR115_1` | R115-1 U10-2 | U10 IO5 → R115 |
| `NetR116_2` | R116-2 U10-13 | U10 IO1 → R116 |
| `NetSD1_1` | SD1-1 | card D2, D1 — unconnected |
| `NetSD1_8` | SD1-8 | card D2, D1 — unconnected |
| `NetType_C1_A8` | Type_C1-A8 | SBU — unconnected |
| `NetType_C2_A8` | Type_C2-A8 | SBU — unconnected |
| `NetType_C1_B8` | Type_C1-B8 | SBU — unconnected |
| `NetType_C2_B8` | Type_C2-B8 | SBU — unconnected |
| `NetU1_10` | U1-10 | CH343 ACT#, DCD#, DSR#, CTS#, RI# — unconnected |
| `NetU1_11` | U1-11 | CH343 ACT#, DCD#, DSR#, CTS#, RI# — unconnected |
| `NetU1_14` | U1-14 | CH343 ACT#, DCD#, DSR#, CTS#, RI# — unconnected |
| `NetU1_15` | U1-15 | CH343 ACT#, DCD#, DSR#, CTS#, RI# — unconnected |
| `NetU1_16` | U1-16 | CH343 ACT#, DCD#, DSR#, CTS#, RI# — unconnected |
| `NetU8_28` | U8-28 | GPIO35, 36, 37 — unconnected (PSRAM) |
| `NetU8_29` | U8-29 | GPIO35, 36, 37 — unconnected (PSRAM) |
| `NetU8_30` | U8-30 | GPIO35, 36, 37 — unconnected (PSRAM) |
| `NetU10_4` | U10-4 | CH422G IO7 — unconnected |
| `NetU10_8` | U10-8 | CH422G OC0 — unconnected |
| `NetU10_9` | U10-9 | CH422G OC1 — unconnected |
| `NetU10_10` | U10-10 | CH422G OC2 — unconnected |
| `NetU10_11` | U10-11 | CH422G OC3 — unconnected |
| `NetU10_12` | U10-12 | CH422G IO0 — unconnected |
