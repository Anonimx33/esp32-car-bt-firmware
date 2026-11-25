#pragma once

#include <Arduino.h>
#include "hw_lib/I2cBus.h"

class Bh1750 {
public:
  explicit Bh1750(I2cBus &bus);

  bool begin(uint8_t address);
  bool read(float &lux);
  bool isReady() const;

private:
  I2cBus &bus_;
  uint8_t address_ = 0x23;
  bool ready_ = false;
};