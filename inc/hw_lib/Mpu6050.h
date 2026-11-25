#pragma once

#include <Arduino.h>
#include "hw_lib/I2cBus.h"

class Mpu6050 {
public:
  struct Sample {
    float ax = 0.0f;
    float ay = 0.0f;
    float az = 0.0f;
    float gx = 0.0f;
    float gy = 0.0f;
    float gz = 0.0f;
    float temperatureC = 0.0f;
  };

  explicit Mpu6050(I2cBus &bus);

  bool begin(uint8_t address);
  bool read(Sample &out);
  bool isReady() const;

private:
  I2cBus &bus_;
  uint8_t address_ = 0x68;
  bool ready_ = false;
};