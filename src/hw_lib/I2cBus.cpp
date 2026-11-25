#include "hw_lib/I2cBus.h"

I2cBus::I2cBus(TwoWire &wire) : wire_(wire) {}

bool I2cBus::begin(uint8_t sda, uint8_t scl, uint32_t frequency) {
  return wire_.begin(sda, scl, frequency);
}

std::vector<uint8_t> I2cBus::scan() {
  std::vector<uint8_t> found;
  for (uint8_t address = 0x08; address < 0x78; ++address) {
    if (isPresent(address)) found.push_back(address);
  }
  return found;
}

bool I2cBus::isPresent(uint8_t address) {
  wire_.beginTransmission(address);
  return wire_.endTransmission() == 0;
}

bool I2cBus::write(uint8_t address, const uint8_t *data, size_t length) {
  wire_.beginTransmission(address);
  wire_.write(data, length);
  return wire_.endTransmission() == 0;
}

bool I2cBus::writeReg(uint8_t address, uint8_t reg, uint8_t value) {
  uint8_t data[2] = {reg, value};
  return write(address, data, sizeof(data));
}

bool I2cBus::read(uint8_t address, uint8_t *buffer, size_t length) {
  size_t received = wire_.requestFrom(address, length);
  if (received != length) return false;
  for (size_t i = 0; i < length; ++i) buffer[i] = wire_.read();
  return true;
}

bool I2cBus::readRegs(uint8_t address, uint8_t reg, uint8_t *buffer, size_t length) {
  wire_.beginTransmission(address);
  wire_.write(reg);
  if (wire_.endTransmission(false) != 0) return false;
  return read(address, buffer, length);
}

TwoWire &I2cBus::wire() {
  return wire_;
}