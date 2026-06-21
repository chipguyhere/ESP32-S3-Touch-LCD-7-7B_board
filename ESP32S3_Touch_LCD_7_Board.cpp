// ESP32S3_Touch_LCD_7_Board - shared I2C bus + CH422G expander for the
// Waveshare ESP32-S3-Touch-LCD-7.
//
// Copyright (c) 2025 chipguyhere
// MIT License

#include "ESP32S3_Touch_LCD_7_Board.h"

// ---------------------------------------------------------------------------
// ThreadsafeWire (self-contained copy; no chipguy_Board dependency)
// ---------------------------------------------------------------------------

ThreadsafeWire::ThreadsafeWire(TwoWire& wire, int sda, int scl)
    : _wire(wire), _sda(sda), _scl(scl),
      _mutex(xSemaphoreCreateRecursiveMutex()), _began(false) {}

void ThreadsafeWire::lock()   { xSemaphoreTakeRecursive(_mutex, portMAX_DELAY); }
void ThreadsafeWire::unlock() { xSemaphoreGiveRecursive(_mutex); }

bool ThreadsafeWire::begin() {
    Guard g(*this);
    if (_began) return true;
    _began = _wire.begin(_sda, _scl);
    return _began;
}

bool ThreadsafeWire::begin(int sda, int scl, uint32_t frequency) {
    Guard g(*this);
    if (_began) return true;
    _sda = sda;
    _scl = scl;
    _began = _wire.begin(sda, scl, frequency);
    return _began;
}

bool ThreadsafeWire::end() {
    Guard g(*this);
    if (!_began) return true;
    bool ok = _wire.end();
    _began = false;
    return ok;
}

bool ThreadsafeWire::setClock(uint32_t freq) { Guard g(*this); return _wire.setClock(freq); }
uint32_t ThreadsafeWire::getClock()          { Guard g(*this); return _wire.getClock(); }

void ThreadsafeWire::setTimeOut(uint16_t ms) { Guard g(*this); _wire.setTimeOut(ms); }
uint16_t ThreadsafeWire::getTimeOut()        { Guard g(*this); return _wire.getTimeOut(); }

void ThreadsafeWire::beginTransmission(uint8_t address) { Guard g(*this); _wire.beginTransmission(address); }
void ThreadsafeWire::beginTransmission(int address)     { Guard g(*this); _wire.beginTransmission(address); }

uint8_t ThreadsafeWire::endTransmission()               { Guard g(*this); return _wire.endTransmission(); }
uint8_t ThreadsafeWire::endTransmission(bool sendStop)  { Guard g(*this); return _wire.endTransmission(sendStop); }

size_t ThreadsafeWire::requestFrom(uint8_t address, size_t size)                  { Guard g(*this); return _wire.requestFrom(address, size); }
size_t ThreadsafeWire::requestFrom(uint8_t address, size_t size, bool sendStop)   { Guard g(*this); return _wire.requestFrom(address, size, sendStop); }
size_t ThreadsafeWire::requestFrom(int address, int size)                         { Guard g(*this); return _wire.requestFrom(address, size); }
size_t ThreadsafeWire::requestFrom(int address, int size, int sendStop)           { Guard g(*this); return _wire.requestFrom(address, size, sendStop); }

size_t ThreadsafeWire::write(uint8_t data)                       { Guard g(*this); return _wire.write(data); }
size_t ThreadsafeWire::write(const uint8_t* data, size_t len)    { Guard g(*this); return _wire.write(data, len); }

int  ThreadsafeWire::available() { Guard g(*this); return _wire.available(); }
int  ThreadsafeWire::read()      { Guard g(*this); return _wire.read(); }
int  ThreadsafeWire::peek()      { Guard g(*this); return _wire.peek(); }
void ThreadsafeWire::flush()     { Guard g(*this); _wire.flush(); }

void ThreadsafeWire::onReceive(void (*cb)(int)) { Guard g(*this); _wire.onReceive(cb); }
void ThreadsafeWire::onRequest(void (*cb)())    { Guard g(*this); _wire.onRequest(cb); }

bool ThreadsafeWire::readRegister(uint8_t address, uint8_t reg, uint8_t* buf, size_t len) {
    Guard g(*this);
    _wire.beginTransmission(address);
    _wire.write(reg);
    if (_wire.endTransmission(false) != 0) return false;
    if (_wire.requestFrom(address, len) != len) return false;
    for (size_t i = 0; i < len; ++i) {
        int b = _wire.read();
        if (b < 0) return false;
        buf[i] = (uint8_t)b;
    }
    return true;
}

bool ThreadsafeWire::readRegister(uint8_t address, uint8_t reg, uint8_t& value) {
    return readRegister(address, reg, &value, 1);
}

bool ThreadsafeWire::writeRegister(uint8_t address, uint8_t reg, const uint8_t* buf, size_t len) {
    Guard g(*this);
    _wire.beginTransmission(address);
    _wire.write(reg);
    if (len > 0) _wire.write(buf, len);
    return _wire.endTransmission() == 0;
}

bool ThreadsafeWire::writeRegister(uint8_t address, uint8_t reg, uint8_t value) {
    return writeRegister(address, reg, &value, 1);
}

bool ThreadsafeWire::writeByte(uint8_t address, uint8_t value) {
    Guard g(*this);
    _wire.beginTransmission(address);
    _wire.write(value);
    return _wire.endTransmission() == 0;
}

bool ThreadsafeWire::readByte(uint8_t address, uint8_t& value) {
    Guard g(*this);
    if (_wire.requestFrom(address, (size_t)1) != 1) return false;
    int b = _wire.read();
    if (b < 0) return false;
    value = (uint8_t)b;
    return true;
}

// ---------------------------------------------------------------------------
// ESP32S3_Touch_LCD_7_Board
// ---------------------------------------------------------------------------

ESP32S3_Touch_LCD_7_Board::ESP32S3_Touch_LCD_7_Board()
    : Wire(::Wire, SDA_PIN, SCL_PIN), _ioShadow(0xFF), _begun(false) {}

bool ESP32S3_Touch_LCD_7_Board::begin() {
    if (_begun) return true;

    Wire.begin();   // ThreadsafeWire on this board's SDA/SCL
    delay(50);

    // Put the CH422G in a known state.  First enable IO0..7 as push-pull outputs
    // (WR_SET bit0), then drive the IO output byte.
    //
    // Idle state: all lines HIGH (reset lines de-asserted) EXCEPT the backlight
    // (IO2), which we leave OFF so the display driver can render a first frame
    // before revealing it -- this avoids showing power-on garbage.  The display
    // driver turns the backlight on (via lcdBacklight) at the end of its begin().
    {
        ThreadsafeWire::Guard g(Wire);
        Wire.writeByte(CH422G_WR_SET, CH422G_BIT_IO_OE);  // enable IO outputs
        _ioShadow = 0xFF & ~(1 << EXIO_LCD_BL);            // all high, backlight off
        Wire.writeByte(CH422G_WR_IO, _ioShadow);
    }

    _begun = true;
    return true;
}

void ESP32S3_Touch_LCD_7_Board::exioWrite(uint8_t io, bool level) {
    if (io > 7) return;
    ThreadsafeWire::Guard g(Wire);   // shadow update + write must be atomic
    if (level) _ioShadow |=  (1 << io);
    else       _ioShadow &= ~(1 << io);
    Wire.writeByte(CH422G_WR_IO, _ioShadow);
}

void ESP32S3_Touch_LCD_7_Board::lcdReset() {
    exioWrite(EXIO_LCD_RST, LOW);
    delay(10);
    exioWrite(EXIO_LCD_RST, HIGH);
    delay(100);
}

void ESP32S3_Touch_LCD_7_Board::lcdBacklight(bool on) {
    exioWrite(EXIO_LCD_BL, on ? HIGH : LOW);
}

void ESP32S3_Touch_LCD_7_Board::touchReset() {
    // Hold INT low while pulsing RST (IO1) low->high so the GT911 latches the
    // 0x5D I2C address, then release INT as an input.
    pinMode(TOUCH_INT_PIN, OUTPUT);
    digitalWrite(TOUCH_INT_PIN, LOW);

    exioWrite(EXIO_TOUCH_RST, LOW);
    delay(10);
    exioWrite(EXIO_TOUCH_RST, HIGH);
    delay(200);

    digitalWrite(TOUCH_INT_PIN, HIGH);
    pinMode(TOUCH_INT_PIN, INPUT);
}
