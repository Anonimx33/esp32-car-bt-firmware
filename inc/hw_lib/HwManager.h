#pragma once

#include <Arduino.h>
#include "hw_lib/I2cBus.h"
#include "hw_lib/Display.h"

class HwManager {
public:
  struct I2cDevice {
    uint8_t address;
    const char *name;
    bool present;
  };

  bool begin();
  void update();
  void printI2cReport();

  I2cBus &i2c();
  Display &display();

private:
  void detectDevices();
  bool isDetected(uint8_t address) const;

  I2cBus i2c_;
  Display display_;
  I2cDevice devices_[5] = {
      {0x3C, "OLED SSD1309", false},
      {0x76, "BME280", false},
      {0x77, "BME280 (alt)", false},
      {0x68, "MPU6050", false},
      {0x23, "BH1750", false},
  };
};