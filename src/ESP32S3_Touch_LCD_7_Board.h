#pragma once

// Board abstraction for the Waveshare ESP32-S3-Touch-LCD-7.
//
// This board hangs several control lines off a CH422G I2C I/O expander rather
// than ESP32 GPIOs: the ST7262 display's reset, the LCD backlight enable, and
// the GT911 touch controller's reset.  Because those lines, the expander, and
// every I2C peripheral on the board share one bus, the bus is wrapped in a
// mutex-protected ThreadsafeWire so tasks and drivers can share it safely.
//
// This class owns that shared bus and the expander, and exposes DEVICE-SPECIFIC
// control methods (lcdReset(), lcdBacklight(), touchReset()) rather than generic
// "toggle EXIO N" calls.  The display and touch libraries take a reference to a
// board instance and drive their hardware through it.
//
// The CH422G is not a register-mapped expander like the TCA9554.  It exposes a
// handful of fixed I2C "command" addresses, each of which is a one-byte write:
//   WR_SET (0x24)  system register: bit0 = enable IO0..7 as push-pull outputs
//   WR_IO  (0x38)  the eight general-purpose IO0..7 output latches (one byte)
//   WR_OC  (0x23)  the four open-drain OC0..3 output latches
//   RD_IO  (0x26)  read IO0..7 inputs
// On this board only the IO0..7 pins are used, so this class keeps a shadow of
// the WR_IO byte and rewrites the whole byte whenever one pin changes.
//
// The ThreadsafeWire wrapper below is intentionally a self-contained copy of the
// one in chipguy_Board (GenericBoard), so this library has no dependency on it.
//
// Copyright (c) 2025 chipguyhere
// MIT License

#include <Arduino.h>
#include <Wire.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

// ThreadsafeWire wraps a TwoWire instance and serializes every call with a
// recursive FreeRTOS mutex, so multiple tasks can safely share one I2C bus.
//
// Single-shot calls (e.g. a stand-alone requestFrom) are safe on their own.
// Multi-step transactions -- most importantly the
//   beginTransmission / write / endTransmission(false) / requestFrom / read...
// "repeated start" pattern used by virtually every register-based driver --
// must hold the mutex across the whole sequence. Use lock()/unlock() or the
// RAII Guard for that. The mutex is recursive, so the inner per-method takes
// nest harmlessly inside an outer Guard.
class ThreadsafeWire {
public:
    ThreadsafeWire(TwoWire& wire, int sda, int scl);

    void lock();
    void unlock();

    class Guard {
    public:
        explicit Guard(ThreadsafeWire& w) : _w(w) { _w.lock(); }
        ~Guard() { _w.unlock(); }
        Guard(const Guard&) = delete;
        Guard& operator=(const Guard&) = delete;
    private:
        ThreadsafeWire& _w;
    };

    // Idempotent. The first call configures the bus on the wired SDA/SCL pins;
    // subsequent calls return true without touching hardware.
    bool begin();
    bool begin(int sda, int scl, uint32_t frequency = 0);
    bool end();

    bool setClock(uint32_t freq);
    uint32_t getClock();

    void setTimeOut(uint16_t ms);
    uint16_t getTimeOut();

    void beginTransmission(uint8_t address);
    void beginTransmission(int address);
    uint8_t endTransmission();
    uint8_t endTransmission(bool sendStop);

    size_t requestFrom(uint8_t address, size_t size);
    size_t requestFrom(uint8_t address, size_t size, bool sendStop);
    size_t requestFrom(int address, int size);
    size_t requestFrom(int address, int size, int sendStop);

    size_t write(uint8_t data);
    size_t write(const uint8_t* data, size_t len);

    int available();
    int read();
    int peek();
    void flush();

    void onReceive(void (*cb)(int));
    void onRequest(void (*cb)());

    // Convenience: register read/write for devices with an 8-bit register
    // index (the overwhelming majority of I2C peripherals). These do the
    // full repeated-start sequence under the mutex, so callers don't have
    // to manage a Guard. Return true on success.
    bool readRegister(uint8_t address, uint8_t reg, uint8_t* buf, size_t len);
    bool readRegister(uint8_t address, uint8_t reg, uint8_t& value);
    bool writeRegister(uint8_t address, uint8_t reg, const uint8_t* buf, size_t len);
    bool writeRegister(uint8_t address, uint8_t reg, uint8_t value);

    // Convenience: single-byte transactions for register-less devices.
    bool writeByte(uint8_t address, uint8_t value);
    bool readByte(uint8_t address, uint8_t& value);

    TwoWire& raw() { return _wire; }

private:
    TwoWire& _wire;
    int _sda;
    int _scl;
    SemaphoreHandle_t _mutex;
    bool _began;
};


class ESP32S3_Touch_LCD_7_Board {
public:
    // Shared I2C bus pins on this board.
    static constexpr int SDA_PIN = 8;
    static constexpr int SCL_PIN = 9;

    // CH422G I2C command addresses (7-bit).  These are not register indices --
    // each is a distinct I2C address that takes a single data byte.
    static constexpr uint8_t CH422G_WR_SET = 0x24;   // 0x48>>1  system register
    static constexpr uint8_t CH422G_WR_OC  = 0x23;   // 0x46>>1  OC0..3 (open drain)
    static constexpr uint8_t CH422G_WR_IO  = 0x38;   // 0x70>>1  IO0..7 outputs
    static constexpr uint8_t CH422G_RD_IO  = 0x26;   // 0x4D>>1  IO0..7 inputs

    // The GT911's INT line is a real GPIO (not on the expander); it is driven
    // during the touch reset to select the controller's I2C address.
    static constexpr int TOUCH_INT_PIN = 4;

    ESP32S3_Touch_LCD_7_Board();

    // Bring up the shared I2C bus and put the expander in its known power-on
    // state (IO0..7 enabled as outputs; reset lines de-asserted; backlight off
    // so the display can draw a first frame before it is revealed).  Idempotent:
    // the display and touch drivers both call it, only the first does the work.
    bool begin();

    // The shared, mutex-protected I2C bus.  Use it directly for any other I2C
    // peripheral on the board; multi-step transactions should hold the bus with
    // a ThreadsafeWire::Guard.
    ThreadsafeWire Wire;

    // --- ST7262 display control lines (on the expander) ---
    void lcdReset();              // pulse the panel reset low->high (IO3)
    void lcdBacklight(bool on);   // LCD backlight enable, active high (IO2)

    // --- GT911 touch control ---
    // Full power-on reset: holds INT low while pulsing the touch reset (IO1),
    // which latches the controller at I2C address 0x5D, then leaves INT as an
    // input.
    void touchReset();

    // --- Escape hatch: generic expander pin control ---
    // io is 0..7 (CH422G IO0..IO7); level is HIGH/LOW.  For lines this class
    // does not name (IO0, IO4=SD_CS, IO5=USB_SEL, IO6, IO7 are otherwise free).
    void exioWrite(uint8_t io, bool level);

private:
    // CH422G WR_SET bits.
    static constexpr uint8_t CH422G_BIT_IO_OE = (1 << 0);   // enable IO outputs

    // CH422G IO pin assignments on this board (0-based IO0..IO7 index).
    static constexpr uint8_t EXIO_TOUCH_RST = 1;   // IO1
    static constexpr uint8_t EXIO_LCD_BL    = 2;   // IO2  (backlight enable)
    static constexpr uint8_t EXIO_LCD_RST   = 3;   // IO3
    static constexpr uint8_t EXIO_SD_CS     = 4;   // IO4
    static constexpr uint8_t EXIO_USB_SEL   = 5;   // IO5

    uint8_t _ioShadow;   // shadow of the CH422G WR_IO output byte
    bool _begun;
};
