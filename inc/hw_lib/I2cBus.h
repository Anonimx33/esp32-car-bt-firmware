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

  TwoWire &wire();

private:
  TwoWire &wire_;
};