#include "hw_lib/Mpu6050.h"

namespace {
constexpr uint8_t REG_SMPLRT_DIV = 0x19;
constexpr uint8_t REG_CONFIG = 0x1A;
constexpr uint8_t REG_GYRO_CONFIG = 0x1B;
constexpr uint8_t REG_ACCEL_CONFIG = 0x1C;
constexpr uint8_t REG_ACCEL_XOUT_H = 0x3B;
constexpr uint8_t REG_PWR_MGMT_1 = 0x6B;
constexpr uint8_t REG_WHO_AM_I = 0x75;

constexpr uint8_t PWR_RESET = 0x80;
constexpr uint8_t PWR_CLOCK_PLL_X = 0x01;
constexpr uint8_t DLPF_44HZ = 0x03;
constexpr uint8_t SAMPLE_DIV_100HZ = 9;
constexpr uint8_t GYRO_250DPS = 0x00;
constexpr uint8_t ACCEL_2G = 0x00;

constexpr float ACCEL_LSB_PER_G = 16384.0f;
constexpr float GYRO_LSB_PER_DPS = 131.0f;

int16_t s16be(const uint8_t *p) { return static_cast<int16_t>((p[0] << 8) | p[1]); }
}

Mpu6050::Mpu6050(I2cBus &bus) : bus_(bus) {}

bool Mpu6050::begin(uint8_t address) {
  address_ = address;
  ready_ = false;

  uint8_t id = 0;
  if (!bus_.readRegs(address_, REG_WHO_AM_I, &id, 1)) return false;
  if (id != 0x68 && id != 0x70 && id != 0x71 && id != 0x73) {
    Serial.printf("[MPU] Nieznany WHO_AM_I 0x%02X\n", id);
    return false;
  }

  bus_.writeReg(address_, REG_PWR_MGMT_1, PWR_RESET);
  delay(100);
  bus_.writeReg(address_, REG_PWR_MGMT_1, PWR_CLOCK_PLL_X);
  delay(10);
  bus_.writeReg(address_, REG_CONFIG, DLPF_44HZ);
  bus_.writeReg(address_, REG_SMPLRT_DIV, SAMPLE_DIV_100HZ);
  bus_.writeReg(address_, REG_GYRO_CONFIG, GYRO_250DPS);
  bus_.writeReg(address_, REG_ACCEL_CONFIG, ACCEL_2G);

  ready_ = true;
  Serial.printf("[MPU] gotowy (0x%02X, WHO_AM_I 0x%02X)\n", address_, id);
  return true;
}

bool Mpu6050::read(Sample &out) {
  if (!ready_) return false;
  uint8_t d[14];
  if (!bus_.readRegs(address_, REG_ACCEL_XOUT_H, d, sizeof(d))) return false;

  out.ax = s16be(&d[0]) / ACCEL_LSB_PER_G;
  out.ay = s16be(&d[2]) / ACCEL_LSB_PER_G;
  out.az = s16be(&d[4]) / ACCEL_LSB_PER_G;
  out.temperatureC = s16be(&d[6]) / 340.0f + 36.53f;
  out.gx = s16be(&d[8]) / GYRO_LSB_PER_DPS;
  out.gy = s16be(&d[10]) / GYRO_LSB_PER_DPS;
  out.gz = s16be(&d[12]) / GYRO_LSB_PER_DPS;
  return true;
}

bool Mpu6050::isReady() const {
  return ready_;
}