# Waveshare ESP32-S3-Touch-LCD-7B — schematic, as understood

> **-7B only, for now.** This file was written from the ESP32-S3-Touch-LCD-7B's
> schematic and bench tests. The ESP32-S3-Touch-LCD-7 (800×480, CH422G expander),
> which this library also supports, has its own schematic; its version of this
> file is **TODO (schematic)**. Don't assume a pin or part here applies to it.

A prose-and-tables reading of Waveshare's schematic for the **ESP32-S3-Touch-LCD-7B**
(7" 1024×600 RGB panel with capacitive touch, ESP32-S3-WROOM-1 module, RS-485, CAN,
microSD, Li-ion charger). The source is `ESP32-S3-Touch-LCD-7B-Schematic.pdf` from
Waveshare's demo package for the board (folder `Waveshare-ESP32-S3-Touch-LCD-7B-Demo`),
an **Altium Designer** export (PDF Creator "Altium Designer", Producer "llPDFLib 3.x",
CreationDate 2025-05-13). It has two pages: page 1 is the single schematic sheet
`ESP32-S3-Touch-LCD-7B.SchDoc`, page 2 the PCB silkscreen / assembly view. The title
block carries only the Waveshare logo and the board name; **no revision or date is
printed on the sheet**, so the PDF's creation date is the only date available, and no
silkscreen revision was compared against it.

**How it was read.** The **netlist was extracted from the PDF's metadata**: Altium
writes every net with its pins, and every component with its pins, into the bookmark
outline, and that tree was dumped with `pypdf`. Connectivity in this file therefore
comes from the netlist and is exact. Net-label aliases (two names on one net, such as
`CTP_IRQ` / `IO4`) were recovered from Altium's hidden marker strings in the page text,
which mangle `_`, `+` and `-` to `0` (`NLCTP0IRQ`, `NLVLED0` for both `VLED+` and
`VLED-`); those were mapped back by hand against the drawing. **Values, part numbers
and stuffing marks are not in the metadata**; they were read from the visible page
text and from renders at 220 dpi (whole sheet) and 400 dpi (zooms), and paired with
reference designators by position. A value can therefore be misattributed even though
the connections cannot. Places where judgement was needed:

- Pairing values to refdes where labels sit between two parts (C1/C2 near the CH343;
  the stacked `x/NC` variants in the backlight block).
- Pin roles (gate/source/drain, anode/cathode, base/emitter/collector) are read from
  the Altium symbols, whose pin numbers are in the netlist.
- **Variant values written `A/NC` or `A/B/NC`** (R20, R21, R22, R28, R32, R48–R52, C34,
  L3) say only which options the layout allows, not which one is fitted. Where the
  fitted option is settled by another part of the circuit or by the demo, the entry
  says so and tags it **inferred**; otherwise it is a **TODO (bench)**.
- The netlist's pin `0` on a part is its exposed pad or mounting tab.

**Labels.** **TODO (schematic)** marks what the sheet doesn't settle, **TODO (bench)**
what only a test settles, **TODO (datasheet)** what a part's datasheet settles, and
**inferred** a conclusion drawn from part numbers, values or the drawing. Facts from
outside the sheet are tagged by source: **(demo)** for Waveshare's Arduino / ESP-IDF
demo code in the same package (`io_extension.*`, `rgb_lcd_port.*`, `gt911.h`,
`05_RS485.ino`, `twai.h`, `pwm.c`), **(table)** for the GPIO assignment table Waveshare
printed on the sheet itself (bottom right: "WROOM-1/2 | LCD | USB | SD | UART | CAN |
Sensor"), **(silkscreen)** for page 2, and **(bench)** for observations of a physical
unit, dated.

**Conventions.** `NET` in code font is a net name as printed; aliases on one net are
written `A / B`. `Ref.pin` is a component pin. Capacitor codes are as printed
(`104` = 100 nF). ESP32-S3 GPIOs are written GPIOn; the module's pin numbers appear only
where they help cross-check the netlist (`U8.27` = GPIO0).

The board library's README and topic files and its `README-engineering.md`, and the
sibling `chipguy_Waveshare-ESP32-S3-Touch-LCD-7-7B_display` library's
`README-engineering.md`, draw on this file. It is the one place that tries to be
**complete rather than useful**; why a circuit is built the way it is belongs in
`README-engineering.md`.

---

## 1. Bill of materials, by function

### 1.1 Processor and module

| Ref | Part | Role |
|---|---|---|
| U8 | **ESP32-S3-WROOM-1-N16R8** | Main MCU module. (bench, 2026-10-26, unit MAC 44:1b:f6:8e:cc:7c): esptool reports ESP32-S3 (QFN56) rev v0.2, 16 MB flash, 8 MB PSRAM. The N16R8's octal PSRAM occupies GPIO35–37, and module pins 28–30 (GPIO35–37) are unconnected on the sheet. |
| C45, C46 | 10 µF, 100 nF | `3V3` decoupling at U8 |
| R93 10 kΩ, C43 100 nF | | `RESET` (= module `EN`, U8.3) pull-up to `3V3` and RC |
| K1 | tact switch, silkscreen **RESET** | `RESET` → GND |
| K2 | tact switch, silkscreen **BOOT** | `G3` (= **GPIO0**) → GND; R100 10 kΩ pull-up to `3V3` |
| R95, R103 | **NC**, **NC** | GPIO3 (`VSYNC`) pull-up / pull-down options — neither fitted |
| R96, R104 | **NC**, **10 kΩ** | GPIO45 (`G4`) pull-up / pull-down: pulled **down** |
| R97, R105 | **NC**, **10 kΩ** | GPIO46 (`HSYNC`) pull-up / pull-down: pulled **down** |

The four resistor pairs sit in a box labelled "Strapping Pins".

### 1.2 Power path

| Ref | Part | Role |
|---|---|---|
| Type_C1 | USB-C receptacle, silkscreen **UART1** | VBUS (A4/B9, A9/B4) → `VUART'`; D+ (A6/B6) `USBD_P`, D− (A7/B7) `USBD_N` → CH343 (§1.4); CC1 (A5) `DCC1`, CC2 (B5) `DCC2`; SBU1/SBU2 (A8/B8) unconnected; GND pins and shell (A1/B12, A12/B1, MTB) on **`GND1`**, a separate ground net |
| R2, R3 | 5.1 kΩ | `DCC1` / `DCC2` to `GND` (not `GND1`): UFP (sink) CC pull-downs |
| TVS1 | LTVS16H5.0ET5G | TVS, `VUART'` → GND |
| L3 | **NC** / ACP3225-501-2P-T000 common-mode choke | 1 `VUART'`, 4 `VUART` (one winding); 2 `GND1`, 3 `GND` (other winding) |
| R28 | 0R/NC | bypass `VUART'` → `VUART` |
| R32 | 0R/NC | bypass `GND1` → `GND` |
| Q5 | AO3401 P-MOSFET | 1 G `NetQ5_1`, 2 S `VBUS`, 3 D `VUART`: ideal diode **`VUART` → `VBUS`** |
| Q7, Q8 | MMBT3906 PNP | Q5's controller. Q7 diode-connected (B = C = `NetQ7_1`, E `VUART`), R23 100 kΩ to GND; Q8 B `NetQ7_1`, E `VBUS`, C → Q5 gate, R24 470 kΩ to GND |
| Q10 | AO3401 P-MOSFET | 1 G `NetQ10_1`, 2 S `5V`, 3 D `VUART`: ideal diode **`VUART` → `5V`** |
| Q11, Q12 | MMBT3906 PNP | Q10's controller. Q11 diode-connected (B = C = `NetQ11_1`, E `VUART`), R67 100 kΩ to GND; Q12 B `NetQ11_1`, E `5V`, C → Q10 gate, R68 470 kΩ to GND |
| D11 | 1N5819 | `VBUS` → `5V` (anode `VBUS`) |
| C1, C2 | 100 nF, 10 µF/16 V | `VBUS` (values read by position; could be swapped) |
| Type_C2 | USB-C receptacle, silkscreen **USB** | VBUS pins **directly on `5V`**; data → §1.5; CC1 `CC1`, CC2 `CC2`; SBU1/SBU2 unconnected; GND pins and shell on `GND` |
| R62, R63 | 5.1 kΩ | `CC1` / `CC2` to GND (sink pull-downs) |
| TVS2 | LTVS16H5.0ET5G | TVS, `5V` → GND |
| J9 | 2-pin connector, silkscreen **POWER GND 5V** | 1 GND, 2 `5V` — a bare 5 V input on the same net as Type_C2 VBUS |
| U4 | **CS8501** Li-ion charger + boost | 1 VOUT `5V`; 2 CHRG# → Led1 cathode; 3 PROG → R64; 4 STDBY# → Led2 cathode; 5 VCC `NetC5_2`; 6 BAT `VBAT`; 7 GND; 8 LX `NetD12_1`; 0 (pad) GND |
| R30, R44 | 0.56 Ω each, parallel | `VBUS` → `NetC5_2` (U4 VCC) |
| C5 | 10 µF/16 V | U4 VCC |
| R64 | **1.2 kΩ** | PROG → GND. The sheet's table: 5 kΩ 160 mA, **1.2 kΩ 580 mA**, 0.75 kΩ 900 mA, 0.68 kΩ 1000 mA |
| L2 | 4.7 µH 2.8 A (4.4×4.2×2) | LX → `VBAT` |
| D12 | 1N5819 | LX → `5V` (the boost output diode) |
| C6, C7 | 10 µF/16 V each | `VBAT` |
| C3, C4 | 100 nF, 100 µF | `5V` bulk at U4 |
| Led1 | green LED, silkscreen **CHARGE** | cathode on CHRG# |
| Led2 | red LED, silkscreen **DONE** | cathode on STDBY# |
| R31 | 4.7 kΩ | common anode of Led1/Led2 ← `NetC5_2` (U4 VCC, i.e. USB-side) |
| J3 | 2-pin battery connector + 2 mounting pads, silkscreen **Battery − / +** | 2 `VBAT`, 1 and 0 GND |
| R18, R19 | 20 kΩ 1 %, 10 kΩ 1 % | `VBAT` → `EXIO_ADC` → GND: VBAT ÷ 3 into U10's ADC (§1.6) |

There is **no power switch**: nothing in the netlist interrupts `VBAT`, U4's VOUT or the
`5V` rail. Inferred from the connectivity:

- **Only Type_C1 can charge the battery.** `VBUS` (the charger's input) is fed only by
  Q5 from `VUART`. Type_C2 and J9 sit on `5V`, which reaches `VBUS` through nothing
  (D11 and Q5's arrangement both point the other way).
- **Type_C2's VBUS pins and J9 pin 2 carry the board's `5V` whenever the board is
  powered** — from Type_C1 (via Q10 and D11) or from the battery boost — since they are
  the same net with no switch or diode.
- The CH343 (U1) and the charge LEDs are powered from `VBUS`, so they are **dead unless
  Type_C1 is plugged in**.
- L3 has **no footprint on the silkscreen** while R28 and R32 do, so the choke is not
  fitted and R28/R32 are the 0 Ω links (inferred, silkscreen). **TODO (bench):**
  confirm `GND1`–`GND` continuity.

### 1.3 Regulators

| Ref | Part | In → out | Notes |
|---|---|---|---|
| U5 | **SGM2212-3.3XKC3G/TR** LDO | `5V` → **`3V3`** | 1 G GND, 2 Vo `3V3`, 3 Vi `5V`. In C8 10 µF/16 V, C9 100 nF; out C10 10 µF/16 V, C11 100 nF. No enable: on whenever `5V` is up. |
| R20 | 0R/NC | `3V3` → `LCD_VDD` | the panel's logic rail (§1.7). `LCD_VDD` has no other source, so R20 is fitted (inferred). Not switchable. |
| U2, U3 | | | panel bias and backlight boosts, §1.8 and §1.9 |

### 1.4 USB-UART, auto-reset and the UART selector

| Ref | Part | Role |
|---|---|---|
| U1 | **CH343P** USB-UART | 1 VIO `3V3`; 2 GND; 3 VDD5 `VBUS`; 4 TXD `CH_RXD`; 5 RXD `CH_TXD`; 6 V3 `3V3`; 7 UD+ `USB_P`; 8 UD− `USB_N`; 9 VBUS `VBUS`; 10 ACT#, 11 DCD#, 14 DSR#, 15 CTS#, 16 RI# unconnected; 12 DTR#/TNOW `DTR`; 13 RTS# `RTS`; 0 (pad) GND |
| R5, R10 | 0 Ω | `USBD_N` → `USB_N`, `USBD_P` → `USB_P` |
| D8, D9 | LESD8LH5.0CT5G | ESD on `USBD_N`, `USBD_P` |
| Q6 | SS8050 NPN | B ← R4 10 kΩ ← `DTR`; E `RTS`; C `RESET` |
| Q9 | SS8050 NPN | B ← R29 10 kΩ ← `RTS`; E `DTR`; C `G3` (GPIO0) |
| U13 | **FSUSB42UMX** 2:1 analog switch | 1 D+ `ESP_RXD` (GPIO44); 2 D− `ESP_TXD` (GPIO43 via R107); 3 GND; 4 HSD1− `CH_TXD`; 5 HSD1+ `CH_RXD`; 6 HSD2− `EX_TXD`; 7 HSD2+ `EX_RXD`; 8 OE# GND (always enabled); 9 VCC `3V3`; 10 SEL `UART_SEL` |
| R107 | 499 Ω | `TX0` (GPIO43, U8.37) → `ESP_TXD` |
| SW1 | 6-pin slide switch (DPDT), silkscreen **UART2 UART1** | 2, 5 `UART_SEL` (commons); 1, 4 GND; 3, 6 `UART_H` |
| R17 | 10 kΩ | `UART_H` pull-up to `3V3` |
| H3 | 4-pin header, silkscreen **UART2 3V3 GND RX TX** | 1 `3V3`, 2 GND, 3 `EX_RXD` (→ GPIO44, the ESP's RX), 4 `EX_TXD` (← GPIO43, the ESP's TX) |

SW1 ties `UART_SEL` to GND or to `UART_H` (pulled high). SEL low selects HSD1, so
**GPIO43/44 reach the CH343** in the "UART1" position; SEL high selects HSD2, so they reach
**H3** in "UART2". (bench, 2026-10-26): the "UART1" position is the CH343. Net names on
the CH343 side are from the ESP's point of view: `CH_RXD` is the CH343's TXD pin.

### 1.5 Native USB / CAN selector

| Ref | Part | Role |
|---|---|---|
| U9 | **FSUSB42UMX** | 1 D+ `ESP_USB_P` (**GPIO20**); 2 D− `ESP_USB_N` (**GPIO19**); 3 GND; 4 HSD1− `ESP_USBH_N`; 5 HSD1+ `ESP_USBH_P`; 6 HSD2− `CANRX`; 7 HSD2+ `CANTX`; 8 OE# GND; 9 VCC `3V3`; 10 SEL `EXIO5` / `USB_SEL` |
| R117 | 10 kΩ | `USB_SEL` pull-down: **USB selected by default**, including before U10 runs |
| R65, R66 | 0 Ω | `ESP_USBH_N/P` → `USBH_N/P` ("close to ENDPOINT") |
| D13, D14 | LESD8LH5.0CT5G | ESD on `USBH_N`, `USBH_P` |
| Type_C2 | | D+ (A6/B6) `USBH_P`, D− (A7/B7) `USBH_N`; power in §1.2 |
| C40 | 100 nF | U9 VCC |

`USB_SEL` high routes **GPIO20 → CAN TXD** and **GPIO19 ← CAN RXD** (U7, §1.14); the demo
uses TX = GPIO20, RX = GPIO19 and drives EXIO5 high for CAN (demo). Native USB
(USB-Serial-JTAG / OTG) and CAN are mutually exclusive.

### 1.6 I/O expander (U10)

| Ref | Part | Role |
|---|---|---|
| U10 | **20-pin MCU, part number not printed** | I²C I/O expander, address **0x24** (demo). Pin names (PA/PC/PD ports, `PD7/NRST`, SWDIO on PD1, I²C on PC1/PC2) and the QFN20 pin order match a **WCH CH32V003F4U6** (inferred). **TODO (schematic):** part identity. |
| C47, C48 | 1 µF, 100 nF | U10 `VDD` |
| TP1 | test pad, silkscreen **SWDIO** | `SWDIO` (U10.15) — the only programming access |
| TP2 | test pad, silkscreen **GND** | GND |
| TP3 | test pad | `EXIO_PWM` |

| U10 pin | Port | Net | Function |
|---|---|---|---|
| 1 | PD7/NRST | `EXIO_RESET` | net label only — connected to nothing else |
| 2, 3 | PA1, PA2 | — | no-connect |
| 4 | VSS | GND | |
| 5 | PD0 | `EXIO0` | net label only — connected to nothing else |
| 6 | VDD | `3V3` | |
| 7 | PC0 | — | no-connect |
| 8 | PC1 | `CTP_SDA` / `SDA` | I²C SDA (GPIO8) |
| 9 | PC2 | `CTP_SCL` / `SCL` | I²C SCL (GPIO9) |
| 10 | PC3 | `EXIO_PWM` | backlight dimming (§1.8), TP3 |
| 11 | PC4 | `EXIO_ADC` | VBAT ÷ 3 (§1.2) |
| 12 | PC5 | `CTP_RST` / `EXIO1` | touch reset (J2.1) |
| 13 | PC6 | `EXIO7` | net label only — connected to nothing else |
| 14 | PC7 | — | no-connect |
| 15 | PD1 | `SWDIO` | TP1 |
| 16 | PD2 | `BL_EN_1` / `DISP` / `EXIO2` | backlight boost enable (U3.4) |
| 17 | PD3 | `EXIO3` / `LCD_RST` | panel reset (J1.44), R11 10 kΩ pull-up to `LCD_VDD` |
| 18 | PD4 | `EXIO4` / `SDCS` | microSD CS (via R92 0 Ω) |
| 19 | PD5 | `EXIO5` / `USB_SEL` | U9 SEL, R117 pull-down |
| 20 | PD6 | `EXIO6` / `LCD_VDD_EN` | panel-bias boost SHDN# (U2.4), R54 10 kΩ pull-up to U2's VIN (5 V) |
| 0 | pad | GND | |

Register protocol (demo): 0x02 direction (bit = 1 output; the demo writes 0xFF), 0x03
output latch, 0x04 input, 0x05 PWM duty (one byte, 0–255), 0x06 ADC (16-bit read).
(bench, 2026-10-26): 0x02, 0x03 and 0x05 read back what was written; 0x04 reads the
live pin levels; register **0x01 reads 0xAA** (an ID byte, inferred); 0x00 and
0x07–0x0F read 0xFF. The registers keep their values across an ESP32 reset, since U10
is not reset with it. The
demo scales the ADC as 10-bit against 3.3 V and multiplies by 3 for VBAT (demo). The
demo's EXIO numbering matches the net names: EXIO1 touch reset, EXIO2 backlight enable,
EXIO3 LCD reset, EXIO4 SD CS, EXIO5 USB/CAN select (0 = USB, 1 = CAN), EXIO6, EXIO7.

### 1.7 Display (J1)

| Ref | Part | Role |
|---|---|---|
| J1 | 50-pin FPC (+ 2 mounting tabs 51, 52), silkscreen "50 … 1"; board silkscreen "1024×600 Pixels" | RGB565 parallel panel, see pin table |
| R27, C35 | 150 Ω series, 8.2 pF to GND | `IO7'` (GPIO7) → `CLK` / `DCLK` / `PCLK` at J1.37 |
| R1 | 10 kΩ | MODE (J1.8) pull-up to `LCD_VDD` |
| R6, R7 | NC (to GND), 0 Ω (to `LCD_VDD`) | L/R (J1.39) tied **high** |
| R8, R9 | NC (to `LCD_VDD`), 0 Ω (to GND) | U/D (J1.40) tied **low** |
| R11 | 10 kΩ | RESET (J1.44, `EXIO3`) pull-up to `LCD_VDD` |
| R12, R13 | 10 kΩ (to GND), NC (to `LCD_VDD`) | DITHB (J1.47) pulled **low** |
| C23, C24 | 10 µF, 100 nF | `3V3` decoupling (drawn beside J2) |

| J1 pin | Name | Net | Source |
|---|---|---|---|
| 1, 2 | VLED+ | `VLED+` | U3 boost output (§1.8) |
| 3, 4 | VLED− | `VLED-` | LED string return → R25‖R26 |
| 5 | GND | GND | |
| 6 | VCOM | `VCOM` | §1.9 |
| 7 | DVDD | `LCD_VDD` | `3V3` via R20 |
| 8 | MODE | `NetJ1_8` | R1 pull-up |
| 9 | DE | `DE` | **GPIO5** |
| 10 | VS | `IO3` / `VS` / `VSYNC` | **GPIO3** |
| 11 | HS | `HS` / `HSYNC` | **GPIO46** |
| 12 | B7 | `B7` | **GPIO10** |
| 13 | B6 | `B6` | **GPIO17** |
| 14 | B5 | `B5` | **GPIO18** |
| 15 | B4 | `B4` | **GPIO38** |
| 16 | B3 | `B3` | **GPIO14** |
| 17, 18, 19 | B2, B1, B0 | GND | |
| 20 | G7 | `G7` | **GPIO21** |
| 21 | G6 | `G6` | **GPIO47** |
| 22 | G5 | `G5` | **GPIO48** |
| 23 | G4 | `G4` | **GPIO45** |
| 24 | G3 | `G3` | **GPIO0** |
| 25 | G2 | `G2` | **GPIO39** |
| 26, 27 | G1, G0 | GND | |
| 28 | R7 | `IO40` / `R7` | **GPIO40** |
| 29 | R6 | `IO41` / `R6` | **GPIO41** |
| 30 | R5 | `IO42` / `R5` | **GPIO42** |
| 31 | R4 | `IO2` / `R4` | **GPIO2** |
| 32 | R3 | `IO1` / `R3` | **GPIO1** |
| 33, 34, 35 | R2, R1, R0 | GND | |
| 36 | GND | GND | |
| 37 | DCLK | `CLK` / `DCLK` / `PCLK` | **GPIO7** via R27 |
| 38 | GND | GND | |
| 39 | L/R | `NetJ1_39` | high (R7) |
| 40 | U/D | `NetJ1_40` | low (R9) |
| 41 | VGH | `VGH` | §1.9 |
| 42 | VGL | `VGL` | §1.9 |
| 43 | AVDD | `AVDD_LCD` | §1.9 |
| 44 | RESET | `EXIO3` / `LCD_RST` | U10 PD3 |
| 45 | NC | `NetJ1_45` | unconnected |
| 46 | VCOM | `VCOM` | |
| 47 | DITHB | `NetJ1_47` | low (R12) |
| 48 | GND | GND | |
| 49, 50 | NC | `NetJ1_49`, `NetJ1_50` | unconnected |
| 51, 52 | (tabs) | GND | |

The panel has **no display-on (DISP) pin** on J1; the sheet's `DISP` label is the
backlight enable. Demo panel timing: 1024 × 600, 30 MHz pixel clock, 16-bit (demo). The
panel's part number and controller are not on the sheet — **TODO (schematic)**.

### 1.8 Backlight

| Ref | Part | Role |
|---|---|---|
| U3 | **AP3032** LED boost | 1 SW `NetD6_1`; 2 GND; 3 FB `NetR48_2`; 4 EN `BL_EN_1`; 5 OV `VLED+`; 6 IN `5V` |
| L6 | 10 µH | `5V` → SW |
| D6 | 1N5819 | SW → `VLED+` |
| C26 | 10 µF | `5V` input |
| C25, C27 | 1 µF/25 V, 104/25 V | `VLED+` |
| R25, R26 | 5.1 Ω each, parallel | `VLED-` → GND: LED current sense |
| R48 | **5.1K/0R/NC** | `VLED-` → FB |
| R21 | **4.7K/NC** | `5V` → `BL_EN_1` |
| R22 | **100K/NC** | `BL_EN_1` → GND |
| R52 | **NC/0R** | `EXIO_PWM` → `BL_EN_1` (PWM on EN) |
| R49 | **0R/NC** | `EXIO_PWM` → `NetR49_2` |
| R50 | **10k/NC** | `NetR49_2` → `NetC34_2` |
| C34 | **100NF/NC** | `NetC34_2` → GND |
| R51 | **68K/NC** | `NetC34_2` → FB (`NetR48_2`) |

The dashed box around R49–R52/C34 offers two ways to dim: PWM straight onto EN (R52),
or a filtered PWM injected into FB (R49, R50, C34, R51, with R48 as the 5.1 kΩ summing
resistor). `BL_EN_1` is also driven directly by U10 PD2, which argues against R52 being
fitted; and the demo treats PWM as inverted (it caps the duty at 97 % "to prevent the
screen from completely turning off", i.e. high duty = dim). (bench, 2026-10-26):
brightness falls steadily as the PWM register goes 0 → 230 and the backlight is dark at
255, with EXIO2 high throughout. That is the FB-injection behaviour, so the **FB path is
fitted and R52 is not** (inferred from bench). **TODO (bench):** confirm the fitted
options of R21, R22 and R48 by inspection.

### 1.9 Panel bias ("Panel_Voltage")

| Ref | Part | Role |
|---|---|---|
| U2 | **AP3012** boost | 1 SW `NetC18_1`; 2 GND; 3 FB `NetR55_1`; 4 SHDN# `EXIO6` / `LCD_VDD_EN`; 5 VIN `NetC36_1` |
| R43 | 0 Ω | `5V` → `NetC36_1` (U2 VIN) |
| C36, C39 | 1 µF, 10 µF | U2 VIN |
| R54 | 10 kΩ | SHDN# pull-up to VIN (5 V): bias **on by default** |
| L1 | 22 µH 4×4 ±20 % 0.9 A | VIN → SW |
| D5 | 1N5819 | SW → `AVDD_LCD` |
| R55, R57 | 33 kΩ, 4.7 kΩ | `AVDD_LCD` → FB → GND. Sheet label: **9.85 V** |
| C31, C32, C33 | 1 µF, 104/50 V, 10 µF/25 V | `AVDD_LCD` |
| C18, D2 | 100 nF, BAT54S | negative charge pump from SW: C18 SW → D2.3; D2.2 GND; D2.1 `NetC14_1` |
| C14, R14 | 1 µF/25 V, 1 kΩ | `NetC14_1` reservoir, → `VGL` |
| D1 | BZT52C6V8S zener | `VGL` clamp to GND (6.8 V) |
| R15, C13, C17 | 10 kΩ, 100 nF, 1 µF/25 V | `VGL` load and filter |
| C19, D3 | 100 nF, BAT54S | positive charge pump from SW: C19 SW → D3.3; D3.1 `AVDD_LCD`; D3.2 `NetC20_1` |
| C20, R41 | 1 µF/25 V, 1 kΩ | `NetC20_1` reservoir, → `VGH`. Sheet label near D4: **16V** |
| D4 | BZT52C18S zener | `VGH` clamp to GND (18 V) |
| R42, C21, C22 | 47 kΩ, 100 nF, 1 µF/25 V | `VGH` load and filter |
| R45 | 15 kΩ | `AVDD_LCD` → `NetR45_1` (top of VCOM divider) |
| R58 | 10 kΩ trim pot, silkscreen **VCOM** | 1 `NetR45_1`, 2 wiper `VCOM`, 3 `NetR56_1` |
| R56 | NC | across R58 (`NetR56_1` – `NetR45_1`) |
| R53 | NC | `NetR45_1` → `VCOM` direct |
| R59 | 6.8 kΩ | `NetR56_1` → GND |
| C37, C38 | 100 nF, 1 µF/25 V | `VCOM`. Sheet note: **"VCOM: 3.1V-5.3V"** |

`VGL` is negative (inferred from the pump's orientation and D1). Both pumps run off U2's
switch node, so **`AVDD_LCD`, `VGH` and `VGL` all switch with `EXIO6`**; `VCOM` follows
`AVDD_LCD`. `LCD_VDD` does not (§1.3).

### 1.10 Touch (J2)

| Ref | Part | Role |
|---|---|---|
| J2 | 6-pin FPC, sheet note "7inch TOUCH (C)" | 1 RST `CTP_RST` (EXIO1); 2 VDD `3V3`; 3 GND; 4 INT `CTP_IRQ` (**GPIO4**); 5 SDA `CTP_SDA`; 6 SCL `CTP_SCL`; 0 (tab) GND |

The controller is on the panel's flex, not this PCB: **GT911**, I²C 0x5D, or 0x14 if INT
is high during reset (demo). No pull-up on `CTP_IRQ` on the board.

### 1.11 The I²C bus

`CTP_SDA` / `SDA` = **GPIO8** (U8.12), `CTP_SCL` / `SCL` = **GPIO9** (U8.17). Pull-ups
R109, R110 4.7 kΩ to `3V3`.

| Device | Where | 7-bit address |
|---|---|---|
| I/O expander | U10 | 0x24 (demo) |
| GT911 touch | J2 | 0x5D / 0x14 (demo) |
| header H1 | silkscreen **I2C 3V3 GND SDA SCL** | 1 `3V3`, 2 GND, 3 SDA, 4 SCL |

### 1.12 TF / microSD (SD1)

| Ref | Part | Role |
|---|---|---|
| SD1 | microSD socket, silkscreen **MICRO SD CARD** | 1 D2 unconnected; 2 CD/D3 `NetR88_1`; 3 CMD `NetR89_1`; 4 VDD `3V3`; 5 CLK `NetR90_1`; 6 VSS GND; 7 D0 `NetR91_2`; 8 D1 unconnected; 9 CD (card-detect switch) **GND**; 10 shell GND |
| R92, R98, R99, R101 | 0 Ω | `EXIO4`/`SDCS` → D3; `IO11`/`MOSI` (**GPIO11**) → CMD; `IO12`/`SCK` (**GPIO12**) → CLK; `IO13`/`MISO` (**GPIO13**) → D0 ("close to SD1") |
| R88, R89, R90, R91 | 10 kΩ | pull-ups to `3V3` on D3, CMD, CLK, D0 (card side) |
| C30, C42 | 100 nF, 10 µF | card VDD |

SPI mode or **1-bit SD mode**: D1 and D2 are not wired, so 4-bit SDMMC is impossible.
In SPI mode **CS is on the I/O expander**, not a GPIO; in SD mode the same line is D3,
which must sit high (R88, and `EXIO4` high). (bench, 2026-10-26): SD_MMC in 1-bit mode on
GPIO12/11/13 mounted a card; Waveshare's demo uses the same mode (demo). The card-detect contact goes to GND and reaches no GPIO:
there is no card detect. Card VDD is the unswitched `3V3`.

### 1.13 RS-485

| Ref | Part | Role |
|---|---|---|
| U6 | **SP3485EN** | 1 RO `IO15` / `RS485_TXD` → **GPIO15**; 2 RE#, 3 DE both `NetQ1_3`; 4 DI `IO16` / `RS485_RXD` ← **GPIO16**; 5 GND; 6 A `A1`; 7 B `B1`; 8 VCC `RS485_VCC` |
| R81, R80 | 0 Ω, NC | `3V3` → `RS485_VCC`; `5V` option not fitted: **U6 runs at 3.3 V** |
| C16 | 100 nF | `RS485_VCC` |
| R74 | 4.7 kΩ | RO pull-up to `RS485_VCC` |
| R79 | 1 kΩ | RE#/DE pull-up to `RS485_VCC` |
| U15 | **SN74LVC1G125DBVR** buffer | 1 OE# GND (always on); 2 A `IO16` (the TX line); 3 GND; 4 Y `NetD7_2`; 5 VCC `3V3`. C50 100 nF. |
| R76, D7 | 200 kΩ, B5819WS | in parallel, Y → `NetC51_2`; D7 anode on the `NetC51_2` side (from the symbol) |
| C51 | 1 nF/50 V | `NetC51_2` → GND |
| Q1 | N-MOSFET, **part number not printed** | 1 G `NetC51_2`; 2 S GND; 3 D `NetQ1_3` (RE#/DE). **TODO (schematic):** part. |
| R82, R73 | 10 kΩ, 10 kΩ | A pull-up to `RS485_VCC`; B pull-down to GND (fail-safe bias) |
| C29, C28 | NC, NC | `RS485_VCC`–A and B–GND options |
| D16 | SMAJ6.5CA | TVS across A–B |
| D15, D17 | SMAJ12CA | TVS B–`EARTH`, A–`EARTH` |
| R86 | 0 Ω | `EARTH` → GND |
| F1, F2 | fuse, **value not printed** | `B1` → `RS485_TX-`, `A1` → `RS485_TX+`. **TODO (schematic):** rating. |
| J7 | 2-pin terminal, silkscreen **RS-485 A B** | 1 `RS485_TX+` (A), 2 `RS485_TX-` (B) |
| R70 | 120 Ω | `B1` → SW2.3 (termination, §1.15) |

**Direction: GPIO16 transmits, GPIO15 receives.** The net labels `RS485_TXD` on GPIO15
and `RS485_RXD` on GPIO16, and the sheet's table ("IO15 RS485_TX, IO16 RS485_RX"), read
the other way, but the netlist puts GPIO15 on U6's RO output and GPIO16 on DI and on the
auto-direction buffer, and the demo agrees: `RS485_RX_PIN 15`, `RS485_TX_PIN 16` (demo).
Direction is automatic: Q1 holds RE#/DE low (receive) while the TX line idles high, and
releases it when TX goes low, D7/R76/C51 holding it released through a character
(inferred). No GPIO controls direction. (bench, 2026-10-26): with the bus idle, GPIO16
follows the ESP32's internal pull-up and pull-down (nothing drives it, as expected of
an input to DI and U15), while GPIO15 reads high under both. GPIO15 alone would read
high from R74 anyway; the decisive half is that GPIO16 floats while the receiver is
enabled, so it cannot be RO.

### 1.14 CAN

| Ref | Part | Role |
|---|---|---|
| U7 | **TJA1051T/3/1J** | 1 TXD `CANTX`; 2 GND `SGND`; 3 VCC `5V`; 4 RXD `CANRX`; 5 VIO `3V3`; 6 CANL; 7 CANH; 8 S `NetR77_2` |
| R77 | 0 Ω | S → `SGND`: **normal mode** permanently (silent mode not controllable) |
| R87 | 0 Ω | `SGND` → GND |
| C12, C15 | 100 nF, 100 nF | `3V3`–`SGND`, `5V`–`SGND` |
| D18 | SM24CANB | 1 CANH, 2 CANL, 3 `SGND` |
| J6 | 2-pin terminal, silkscreen **CAN L H**; sheet "High / Low" | 1 `CANL`, 2 `CANH` |
| R75 | 120 Ω | `CANH` → SW2.1 (termination) |

`CANTX`/`CANRX` reach GPIO20/GPIO19 only through U9 with `USB_SEL` high (§1.5).

### 1.15 Termination switch

| Ref | Part | Role |
|---|---|---|
| SW2 | 2-position DIP, silkscreen **120R ON/OFF, CAN / RS-485** | 1 `NetR75_2` – 4 `CANL`: CAN termination (R75 across CANH–CANL); 2 `A1` – 3 `NetR70_1`: RS-485 termination (R70 across A–B) |

### 1.16 LED, headers, test points

| Ref | Part | Role |
|---|---|---|
| PWR1 | red LED, silkscreen **POWER** | `3V3` → R106 1 kΩ → PWR1 → GND: on whenever `3V3` is |
| J8 | 3-pin header, silkscreen **GPIO 3V3 GND GP6** | 1 `3V3`, 2 GND, 3 `GPIO6` (**GPIO6**); the table lists it under "Sensor" (table) |
| H1, H3, J6, J7, J9 | | §1.11, §1.4, §1.14, §1.13, §1.2 |
| TP1–TP3 | | §1.6 |
| logo1 | logo | no pins |

---

## 2. The complete GPIO assignment

| GPIO | Module pin | Net | Use | Notes |
|---|---|---|---|---|
| 0 | 27 | `G3` | panel G3 (J1.24); BOOT K2; CH343 auto-boot (Q9) | strapping; R100 10 kΩ pull-up |
| 1 | 39 | `IO1` / `R3` | panel R3 | |
| 2 | 38 | `IO2` / `R4` | panel R4 | |
| 3 | 15 | `IO3` / `VSYNC` | panel VSYNC | strapping; R95/R103 both NC |
| 4 | 4 | `CTP_IRQ` | GT911 INT (J2.4) | |
| 5 | 5 | `DE` | panel DE | |
| 6 | 6 | `GPIO6` | **J8 pin 3** | **free** — the only GPIO with nothing else on it |
| 7 | 7 | `IO7'` → `PCLK` | panel DCLK via R27 150 Ω | |
| 8 | 12 | `CTP_SDA` / `SDA` | I²C SDA | U10, GT911, H1 |
| 9 | 17 | `CTP_SCL` / `SCL` | I²C SCL | |
| 10 | 18 | `B7` | panel B7 | |
| 11 | 19 | `IO11` / `MOSI` | microSD CMD via R98 | |
| 12 | 20 | `IO12` / `SCK` | microSD CLK via R99 | |
| 13 | 21 | `IO13` / `MISO` | microSD D0 via R101 | |
| 14 | 22 | `B3` | panel B3 | |
| 15 | 8 | `IO15` | RS-485 **RX** (U6 RO) | R74 4.7 kΩ pull-up; driven by U6 whenever it is receiving |
| 16 | 9 | `IO16` | RS-485 **TX** (U6 DI, U15 A) | |
| 17 | 10 | `B6` | panel B6 | |
| 18 | 11 | `B5` | panel B5 | |
| 19 | 13 | `ESP_USB_N` | U9 D−: Type_C2 D− **or** CAN RXD | per `USB_SEL` (EXIO5) |
| 20 | 14 | `ESP_USB_P` | U9 D+: Type_C2 D+ **or** CAN TXD | per `USB_SEL` |
| 21 | 23 | `G7` | panel G7 | |
| 35, 36, 37 | 28, 29, 30 | — | unconnected | used by the N16R8's octal PSRAM; not available |
| 38 | 31 | `B4` | panel B4 | |
| 39 | 32 | `G2` | panel G2 | |
| 40 | 33 | `IO40` / `R7` | panel R7 | |
| 41 | 34 | `IO41` / `R6` | panel R6 | |
| 42 | 35 | `IO42` / `R5` | panel R5 | |
| 43 | 37 | `TX0` → `ESP_TXD` | UART0 TX via R107 499 Ω → U13 → CH343 **or** H3.4 | per SW1 |
| 44 | 36 | `ESP_RXD` | UART0 RX ← U13 ← CH343 **or** H3.3 | per SW1 |
| 45 | 26 | `G4` | panel G4 | strapping; R104 10 kΩ pull-down (R96 NC) |
| 46 | 16 | `HS` / `HSYNC` | panel HSYNC | strapping; R105 10 kΩ pull-down (R97 NC) |
| 47 | 24 | `G6` | panel G6 | |
| 48 | 25 | `G5` | panel G5 | |
| EN | 3 | `RESET` | K1, Q6, R93/C43 | |

Every exposed GPIO except **GPIO6** is committed. Without giving up a function, the
only other pins a sketch can have are **GPIO43/44 on H3** (SW1 in "UART2", losing the
CH343) and **GPIO19/20** (via U9, as USB or as CAN). GPIO15/16 are free only if the
RS-485 transceiver is tolerated on them: U6 drives GPIO15 whenever it is in receive.

Waveshare's table (table) agrees with every row above. It also prints "RTCN" / "RTCP"
in its "Sensor" column against IO15 / IO16, which nothing on the sheet explains —
**TODO (schematic)**.

Datasheet notes (ESP32-S3 datasheet, not the sheet): strapping pins are **GPIO0**
(boot mode, with GPIO46: download needs GPIO0 = 0 and GPIO46 = 0), **GPIO3** (JTAG
source, only when eFuses select it), **GPIO45** (VDD_SPI voltage; low = 3.3 V, which the
module's 3.3 V flash needs) and **GPIO46** (boot mode / ROM log). Here GPIO0 is pulled
up with a button to ground, GPIO45 and GPIO46 are pulled down, GPIO3 floats. ADC1 is
GPIO1–10 and ADC2 GPIO11–20, so GPIO6 is **ADC1_CH5**. Touch channels are GPIO1–14.
RTC (deep-sleep-capable) GPIOs are GPIO0–21. The pad JTAG pins GPIO39–42 are panel data
lines; USB-Serial-JTAG on GPIO19/20 works only with U9 on USB.

### I/O expander lines

| EXIO | U10 pin | Net | Use |
|---|---|---|---|
| EXIO0 | 5 (PD0) | `EXIO0` | unconnected |
| EXIO1 | 12 (PC5) | `CTP_RST` | GT911 reset |
| EXIO2 | 16 (PD2) | `BL_EN_1` / `DISP` | AP3032 EN (backlight on/off) |
| EXIO3 | 17 (PD3) | `LCD_RST` | panel reset |
| EXIO4 | 18 (PD4) | `SDCS` | microSD CS |
| EXIO5 | 19 (PD5) | `USB_SEL` | U9: 0 = USB, 1 = CAN |
| EXIO6 | 20 (PD6) | `LCD_VDD_EN` | AP3012 SHDN# (panel bias) |
| EXIO7 | 13 (PC6) | `EXIO7` | unconnected |
| PWM | 10 (PC3) | `EXIO_PWM` | backlight dimming, TP3 |
| ADC | 11 (PC4) | `EXIO_ADC` | VBAT ÷ 3 |

## 3. Power rails

| Rail | Source | Switchable? | Feeds |
|---|---|---|---|
| `VUART'` | Type_C1 VBUS | — | TVS1; → `VUART` via R28 (or L3) |
| `VUART` | `VUART'` | — | Q5 → `VBUS`; Q10 → `5V` |
| `VBUS` | `VUART` via Q5 ideal diode | no — Type_C1 only | CH343 VDD5 and VBUS pins; charger VCC via R30‖R44; charge-LED anodes; D11 → `5V` |
| `5V` | Type_C2 VBUS (direct); J9; Q10 from `VUART`; D11 from `VBUS`; U4 VOUT / D12 (battery boost) | no | U5 LDO; U3 backlight boost; U2 panel bias (via R43); U7 CAN VCC; R21 option; R80 option (NC) |
| `VBAT` | J3 cell; U4 charger | no | U4 BAT / boost; R18/R19 divider (always loaded) |
| `3V3` | U5 SGM2212-3.3 | no (no EN) | U8, U10, U1 VIO/V3, U9, U13, U15, U7 VIO, RS-485 (via R81), microSD, touch, H1, H3, J8, PWR1, pull-ups, `LCD_VDD` |
| `LCD_VDD` | `3V3` via R20 | no | panel DVDD (J1.7), MODE/L/R/RESET pull-ups |
| `AVDD_LCD` (9.85 V) | U2 AP3012 | **EXIO6** (SHDN#, default on via R54) | J1.43; VGH pump; VCOM divider |
| `VGH` (label "16V") | D3 charge pump from U2 SW | with EXIO6 | J1.41 |
| `VGL` (negative, 6.8 V zener) | D2 charge pump from U2 SW | with EXIO6 | J1.42 |
| `VCOM` | R45/R58/R59 divider from `AVDD_LCD` | with EXIO6 | J1.6, J1.46 |
| `VLED+` | U3 AP3032 | **EXIO2** (EN); dimmed by EXIO_PWM | panel backlight J1.1–2 |
| `RS485_VCC` | `3V3` via R81 0 Ω | no | U6, its bias and pull-ups |
| `SGND` | GND via R87 0 Ω | — | U7 ground, D18 |
| `EARTH` | GND via R86 0 Ω | — | RS-485 TVS D15/D17 |
| `GND1` | Type_C1 ground and shell; GND via R32 (or L3) | — | Type_C1 |

## 4. What the sheet does not say

- **U10's part number** — not printed; the pinout matches a CH32V003F4U6 (inferred). Its
  firmware, register map and ADC resolution come from the demo only. Whether its I/O
  tolerate R54's pull-up of EXIO6 to 5 V through 10 kΩ — **TODO (datasheet)** once the
  part is known.
- **Q1's part** and **F1/F2's rating** — not printed. **TODO (schematic).**
- **Which variant parts are fitted** in the backlight block (R21, R22, R48–R52, C34):
  §1.8 gives the inference; **TODO (bench).**
- **The panel**: glass, controller and timing are not on the sheet; the demo uses
  1024 × 600 at 30 MHz, 16-bit (demo). The meaning of MODE high, L/R high, U/D low and
  DITHB low comes from the panel datasheet — **TODO (datasheet).**
- **VCOM range**: the printed note "3.1V-5.3V" does not match the divider as drawn
  (R45 15 kΩ, R58 10 kΩ, R59 6.8 kΩ from 9.85 V give roughly 2.1–5.2 V). Either a value
  is misread or the note predates a change. **TODO (bench):** measure `VCOM` at both
  ends of R58.
- **CS8501 behaviour** — termination voltage, whether the boost runs whenever a cell is
  present, boost current limit, and how charging and boosting share `5V` — comes from
  the part's datasheet. **TODO (datasheet).** The 580 mA charge current is the sheet's
  own table for R64 = 1.2 kΩ.
- **The battery cell**: J3 is a bare connector; chemistry, capacity and protection PCB
  are not on the sheet.
- **CH343P V3 pin** is tied to the board's `3V3` while VDD5 is on `VBUS`; whether that
  matches the CH343 datasheet's recommendation, and what the CH343 does to the UART
  lines when unpowered (Type_C1 unplugged) — **TODO (datasheet / bench).**
- **SW1 position names**: (bench, 2026-10-26) "UART1" = CH343. The silkscreen "UART2"
  side = H3 follows from the netlist.
- **"RTCN" / "RTCP"** in the sheet's table against IO15/IO16 — unexplained, **TODO
  (schematic).**
- **Current draw** in any state — not measured. **TODO (bench).**

---

## Appendix A. Named nets

Pins are `Ref-pin` as the Altium outline gives them; `0` is an exposed pad or mounting
tab. Aliases come from the hidden net-label markers (§ opening paragraph).

| Net (aliases) | Pins |
|---|---|
| `3V3` | C10-2 C11-2 C12-2 C23-2 C24-2 C30-2 C40-2 C42-2 C45-2 C46-2 C47-1 C48-1 C50-1 H1-1 H3-1 J2-2 J8-1 R17-1 R20-2 R81-2 R88-2 R89-2 R90-2 R91-1 R93-2 R95-1 R96-1 R97-1 R100-2 R106-1 R109-1 R110-1 SD1-4 U1-1 U1-6 U5-2 U7-5 U8-2 U9-9 U10-6 U13-9 U15-5 |
| `5V` | C3-2 C4-2 C8-2 C9-2 C15-2 C26-2 D11-2 D12-2 J9-2 L6-1 Q10-2 Q12-2 R21-1 R43-1 R80-2 TVS2-1 Type_C2-A4/B9 Type_C2-A9/B4 U3-6 U4-1 U5-3 U7-3 |
| `A1` | C29-2 D16-2 D17-1 F2-2 R82-2 SW2-2 U6-6 |
| `AVDD_LCD` | C31-1 C32-2 C33-2 D3-1 D5-2 J1-43 R45-2 R55-2 |
| `B1` | C28-1 D15-2 D16-1 F1-2 R70-2 R73-1 U6-7 |
| `B3` / `IO14` | J1-16 U8-22 |
| `B4` / `IO38` | J1-15 U8-31 |
| `B5` / `IO18` | J1-14 U8-11 |
| `B6` / `IO17` | J1-13 U8-10 |
| `B7` / `IO10` | J1-12 U8-18 |
| `BL_EN_1` / `DISP` / `EXIO2` | R21-2 R22-1 R52-2 U3-4 U10-16 |
| `CANH` | D18-1 J6-2 R75-1 U7-7 |
| `CANL` | D18-2 J6-1 SW2-4 U7-6 |
| `CANRX` | U7-4 U9-6 |
| `CANTX` | U7-1 U9-7 |
| `CC1` | R62-2 Type_C2-A5 |
| `CC2` | R63-2 Type_C2-B5 |
| `CH_RXD` | U1-4 U13-5 |
| `CH_TXD` | U1-5 U13-4 |
| `CLK` / `DCLK` / `IO7` / `PCLK` | C35-2 J1-37 R27-1 |
| `CTP_IRQ` / `IO4` | J2-4 U8-4 |
| `CTP_RST` / `EXIO1` | J2-1 U10-12 |
| `CTP_SCL` / `IO9` / `SCL` | H1-4 J2-6 R110-2 U8-17 U10-9 |
| `CTP_SDA` / `IO8` / `SDA` | H1-3 J2-5 R109-2 U8-12 U10-8 |
| `DCC1` | R2-2 Type_C1-A5 |
| `DCC2` | R3-2 Type_C1-B5 |
| `DE` / `IO5` | J1-9 U8-5 |
| `DTR` | Q9-2 R4-2 U1-12 |
| `EARTH` | D15-1 D17-2 R86-2 |
| `ESP_RXD` / `RX0` | U8-36 U13-1 |
| `ESP_TXD` | R107-1 U13-2 |
| `ESP_USBH_N` | R65-1 U9-4 |
| `ESP_USBH_P` | R66-1 U9-5 |
| `ESP_USB_N` | U8-13 U9-2 |
| `ESP_USB_P` | U8-14 U9-1 |
| `EXIO0` | U10-5 |
| `EXIO3` / `LCD_RST` | J1-44 R11-1 U10-17 |
| `EXIO4` / `SDCS` | R92-2 U10-18 |
| `EXIO5` / `USB_SEL` | R117-1 U9-10 U10-19 |
| `EXIO6` / `LCD_VDD_EN` | R54-2 U2-4 U10-20 |
| `EXIO7` | U10-13 |
| `EXIO_ADC` | R18-1 R19-2 U10-11 |
| `EXIO_PWM` | R49-1 R52-1 TP3-1 U10-10 |
| `EXIO_RESET` | U10-1 |
| `EX_RXD` | H3-3 U13-7 |
| `EX_TXD` | H3-4 U13-6 |
| `G2` / `IO39` | J1-25 U8-32 |
| `G3` / `IO0` | J1-24 K2-1 Q9-3 R100-1 U8-27 |
| `G4` / `IO45` | J1-23 R96-2 R104-1 U8-26 |
| `G5` / `IO48` | J1-22 U8-25 |
| `G6` / `IO47` | J1-21 U8-24 |
| `G7` / `IO21` | J1-20 U8-23 |
| `GND` | C1-1 C2-2 C3-1 C4-1 C5-1 C6-1 C7-1 C8-1 C9-1 C10-1 C11-1 C13-2 C14-2 C16-1 C17-2 C20-2 C21-2 C22-2 C23-1 C24-1 C25-1 C26-1 C27-1 C28-2 C30-1 C31-2 C32-1 C33-1 C34-1 C35-1 C36-2 C37-2 C38-2 C39-2 C40-1 C42-1 C43-1 C45-1 C46-1 C47-2 C48-2 C50-2 C51-1 D1-2 D2-2 D4-1 D8-2 D9-2 D13-2 D14-2 H1-2 H3-2 J1-5 J1-17 J1-18 J1-19 J1-26 J1-27 J1-33 J1-34 J1-35 J1-36 J1-38 J1-48 J1-51 J1-52 J2-0 J2-3 J3-0 J3-1 J8-2 J9-1 K1-1 K2-2 L3-3 PWR1-K Q1-2 R2-1 R3-1 R6-2 R9-2 R12-2 R15-2 R19-1 R22-2 R23-2 R24-2 R25-1 R26-1 R32-1 R42-2 R57-1 R59-1 R62-1 R63-1 R64-1 R67-2 R68-2 R73-2 R86-1 R87-1 R103-2 R104-2 R105-2 R117-2 SD1-6 SD1-9 SD1-10 SW1-1 SW1-4 TP2-1 TVS1-2 TVS2-2 Type_C2-A1/B12 Type_C2-A12/B1 Type_C2-MTB U1-0 U1-2 U2-2 U3-2 U4-0 U4-7 U5-1 U6-5 U8-1 U8-40 U8-41 U9-3 U9-8 U10-0 U10-4 U13-3 U13-8 U15-1 U15-3 |
| `GND1` | L3-2 R32-2 Type_C1-A1/B12 Type_C1-A12/B1 Type_C1-MTB |
| `GPIO6` / `IO6` | J8-3 U8-6 |
| `HS` / `HSYNC` / `IO46` | J1-11 R97-2 R105-1 U8-16 |
| `IO1` / `R3` | J1-32 U8-39 |
| `IO2` / `R4` | J1-31 U8-38 |
| `IO3` / `VS` / `VSYNC` | J1-10 R95-2 R103-1 U8-15 |
| `IO7'` | R27-2 U8-7 |
| `IO11` / `MOSI` | R98-2 U8-19 |
| `IO12` / `SCK` | R99-2 U8-20 |
| `IO13` / `MISO` | R101-2 U8-21 |
| `IO15` / `RS485_TXD` | R74-1 U6-1 U8-8 |
| `IO16` / `RS485_RXD` | U6-4 U8-9 U15-2 |
| `IO40` / `R7` | J1-28 U8-33 |
| `IO41` / `R6` | J1-29 U8-34 |
| `IO42` / `R5` | J1-30 U8-35 |
| `LCD_VDD` | J1-7 R1-2 R7-2 R8-2 R11-2 R13-2 R20-1 |
| `RESET` | C43-2 K1-2 Q6-3 R93-1 U8-3 |
| `RS485_TX+` | F2-1 J7-1 |
| `RS485_TX-` | F1-1 J7-2 |
| `RS485_VCC` | C16-2 C29-1 R74-2 R79-2 R80-1 R81-1 R82-1 U6-8 |
| `RTS` | Q6-2 R29-2 U1-13 |
| `SGND` | C12-1 C15-1 D18-3 R77-1 R87-2 U7-2 |
| `SWDIO` | TP1-1 U10-15 |
| `TX0` | R107-2 U8-37 |
| `UART_H` | R17-2 SW1-3 SW1-6 |
| `UART_SEL` | SW1-2 SW1-5 U13-10 |
| `USBD_N` | D8-1 R5-2 Type_C1-A7 Type_C1-B7 |
| `USBD_P` | D9-1 R10-2 Type_C1-A6 Type_C1-B6 |
| `USBH_N` | D13-1 R65-2 Type_C2-A7 Type_C2-B7 |
| `USBH_P` | D14-1 R66-2 Type_C2-A6 Type_C2-B6 |
| `USB_N` | R5-1 U1-8 |
| `USB_P` | R10-1 U1-7 |
| `VBAT` | C6-2 C7-2 J3-2 L2-2 R18-2 U4-6 |
| `VBUS` | C1-2 C2-1 D11-1 Q5-2 Q8-2 R30-2 R44-2 U1-3 U1-9 |
| `VCOM` | C37-1 C38-1 J1-6 J1-46 R53-1 R58-2 |
| `VGH` | C21-1 C22-1 D4-2 J1-41 R41-1 R42-1 |
| `VGL` | C13-1 C17-1 D1-1 J1-42 R14-1 R15-1 |
| `VLED+` | C25-2 C27-2 D6-2 J1-1 J1-2 U3-5 |
| `VLED-` | J1-3 J1-4 R25-2 R26-2 R48-1 |
| `VUART` | L3-4 Q5-3 Q7-2 Q10-3 Q11-2 R28-1 |
| `VUART'` | L3-1 R28-2 TVS1-1 Type_C1-A4/B9 Type_C1-A9/B4 |

## Appendix B. Unnamed nets

Altium's automatic names (`Net<Ref>_<pin>`), with a description. Single-pin entries are
unconnected pins.

| Net | Pins | What it is |
|---|---|---|
| `NetC14_1` | C14-1 D2-1 R14-2 | VGL pump reservoir |
| `NetC18_1` | C18-1 C19-1 D5-1 L1-2 U2-1 | U2 switch node |
| `NetC18_2` | C18-2 D2-3 | VGL pump flying node |
| `NetC19_2` | C19-2 D3-3 | VGH pump flying node |
| `NetC20_1` | C20-1 D3-2 R41-2 | VGH pump reservoir |
| `NetC34_2` | C34-2 R50-2 R51-1 | backlight PWM filter node |
| `NetC36_1` | C36-1 C39-1 L1-1 R43-2 R54-1 U2-5 | U2 VIN (5 V via R43) |
| `NetC51_2` | C51-2 D7-1 Q1-1 R76-2 | Q1 gate (RS-485 auto-direction) |
| `NetC5_2` | C5-2 R30-1 R31-2 R44-1 U4-5 | U4 VCC |
| `NetD12_1` | D12-1 L2-1 U4-8 | U4 LX |
| `NetD6_1` | D6-1 L6-2 U3-1 | U3 switch node |
| `NetD7_2` | D7-2 R76-1 U15-4 | U15 output |
| `NetJ1_39` | J1-39 R6-1 R7-1 | panel L/R strap |
| `NetJ1_40` | J1-40 R8-1 R9-1 | panel U/D strap |
| `NetJ1_47` | J1-47 R12-1 R13-1 | panel DITHB strap |
| `NetJ1_8` | J1-8 R1-1 | panel MODE strap |
| `NetLed1_A` | Led1-A Led2-A R31-1 | charge-LED anodes |
| `NetLed1_K` | Led1-K U4-2 | CHRG# |
| `NetLed2_K` | Led2-K U4-4 | STDBY# |
| `NetPWR1_A` | PWR1-A R106-2 | power LED |
| `NetQ10_1` | Q10-1 Q12-3 R68-1 | Q10 gate |
| `NetQ11_1` | Q11-1 Q11-3 Q12-1 R67-1 | Q11/Q12 bases |
| `NetQ1_3` | Q1-3 R79-1 U6-2 U6-3 | U6 RE#/DE |
| `NetQ5_1` | Q5-1 Q8-3 R24-1 | Q5 gate |
| `NetQ6_1` | Q6-1 R4-1 | Q6 base |
| `NetQ7_1` | Q7-1 Q7-3 Q8-1 R23-1 | Q7/Q8 bases |
| `NetQ9_1` | Q9-1 R29-1 | Q9 base |
| `NetR45_1` | R45-1 R53-2 R56-2 R58-1 | VCOM divider top |
| `NetR48_2` | R48-2 R51-2 U3-3 | U3 FB |
| `NetR49_2` | R49-2 R50-1 | backlight PWM filter |
| `NetR55_1` | R55-1 R57-2 U2-3 | U2 FB |
| `NetR56_1` | R56-1 R58-3 R59-2 | VCOM divider bottom |
| `NetR64_2` | R64-2 U4-3 | U4 PROG |
| `NetR70_1` | R70-1 SW2-3 | RS-485 termination |
| `NetR75_2` | R75-2 SW2-1 | CAN termination |
| `NetR77_2` | R77-2 U7-8 | U7 S |
| `NetR88_1` | R88-1 R92-1 SD1-2 | card CD/D3 (CS) |
| `NetR89_1` | R89-1 R98-1 SD1-3 | card CMD |
| `NetR90_1` | R90-1 R99-1 SD1-5 | card CLK |
| `NetR91_2` | R91-2 R101-1 SD1-7 | card D0 |
| `NetJ1_45`, `NetJ1_49`, `NetJ1_50` | J1-45, J1-49, J1-50 | panel NC pins |
| `NetSD1_1`, `NetSD1_8` | SD1-1, SD1-8 | card D2, D1 — unconnected |
| `NetType_C1_A8`, `NetType_C1_B8` | Type_C1-A8, -B8 | SBU — unconnected |
| `NetType_C2_A8`, `NetType_C2_B8` | Type_C2-A8, -B8 | SBU — unconnected |
| `NetU10_2`, `NetU10_3`, `NetU10_7`, `NetU10_14` | U10-2, -3, -7, -14 | PA1, PA2, PC0, PC7 — unconnected |
| `NetU1_10`, `NetU1_11`, `NetU1_14`, `NetU1_15`, `NetU1_16` | U1-10, -11, -14, -15, -16 | CH343 ACT#, DCD#, DSR#, CTS#, RI# — unconnected |
| `NetU8_28`, `NetU8_29`, `NetU8_30` | U8-28, -29, -30 | GPIO35, 36, 37 — unconnected (PSRAM) |
