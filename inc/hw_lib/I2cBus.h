#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <vector>

class I2cBus {
public:
  explicit I2cBus(TwoWire &wire = Wire);

  bool begin(uint8_t sda, uint8_t scl, uint32_t frequency);
  std::vector<uint8_t> scan();
  bool isPresent(uint8_t address);

  bool write(uint8_t address, const uint8_t *data, size_t length);
  bool writeReg(uint8_t address, uint8_t reg, uint8_t value);
  bool read(uint8_t address, uint8_t *buffer, size_t length);
  bool readRegs(uint8_t address, uint8_t reg, uint8_t *buffer, size_t length);

  TwoWire &wire();

private:
  TwoWire &wire_;
};