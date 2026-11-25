#pragma once

#include <Arduino.h>
#include "hw_lib/I2cBus.h"
#include "hw_lib/Display.h"
#include "hw_lib/Bme280.h"
#include "hw_lib/Bh1750.h"
#include "hw_lib/Mpu6050.h"
#include "hw_lib/Encoder.h"

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
  Encoder &encoder();

  const Bme280::Reading &environment() const;
  bool hasHumidity() const;
  bool hasLight() const;
  float lux() const;

  bool hasImu() const;
  uint32_t imuSequence() const;
  const Mpu6050::Sample &imuSample() const;

private:
  void detectDevices();
  bool isDetected(uint8_t address) const;
  void updateSensors(uint32_t now);

  I2cBus i2c_;
  Display display_;
  Bme280 bme_{i2c_};
  Bh1750 light_{i2c_};
  Mpu6050 imu_{i2c_};
  Encoder encoder_;

  Bme280::Reading environment_;
  float lux_ = 0.0f;
  bool luxValid_ = false;
  Mpu6050::Sample imuSample_;
  uint32_t imuSequence_ = 0;

  uint32_t lastBmeRead_ = 0;
  uint32_t lastLightRead_ = 0;
  uint32_t lastImuRead_ = 0;

  I2cDevice devices_[6] = {
      {0x3C, "OLED SSD1309", false},
      {0x76, "BME280", false},
      {0x77, "BME280 (alt)", false},
      {0x68, "MPU6050", false},
      {0x69, "MPU6050 (alt)", false},
      {0x23, "BH1750", false},
  };
};