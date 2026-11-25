#pragma once

#include <Arduino.h>
#include "hw_lib/I2cBus.h"

class Bme280 {
public:
  struct Reading {
    bool valid = false;
    float temperatureC = 0.0f;
    float humidity = 0.0f;
    float pressureHpa = 0.0f;
  };

  explicit Bme280(I2cBus &bus);

  bool begin(uint8_t address);
  bool read(Reading &out);
  bool hasHumidity() const;
  bool isReady() const;

private:
  bool readCalibration();
  int32_t compensateTemperature(int32_t adc);
  uint32_t compensatePressure(int32_t adc) const;
  uint32_t compensateHumidity(int32_t adc) const;

  I2cBus &bus_;
  uint8_t address_ = 0x76;
  bool ready_ = false;
  bool hasHumidity_ = false;
  int32_t tFine_ = 0;

  uint16_t digT1_ = 0;
  int16_t digT2_ = 0, digT3_ = 0;
  uint16_t digP1_ = 0;
  int16_t digP2_ = 0, digP3_ = 0, digP4_ = 0, digP5_ = 0;
  int16_t digP6_ = 0, digP7_ = 0, digP8_ = 0, digP9_ = 0;
  uint8_t digH1_ = 0, digH3_ = 0;
  int16_t digH2_ = 0, digH4_ = 0, digH5_ = 0;
  int8_t digH6_ = 0;
};