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

TwoWire &I2cBus::wire() {
  return wire_;
}