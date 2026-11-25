#include "hw_lib/Bh1750.h"

namespace {
constexpr uint8_t CMD_POWER_ON = 0x01;
constexpr uint8_t CMD_RESET = 0x07;
constexpr uint8_t CMD_CONT_HIGH_RES = 0x10;
constexpr float COUNTS_PER_LUX = 1.2f;
}

Bh1750::Bh1750(I2cBus &bus) : bus_(bus) {}

bool Bh1750::begin(uint8_t address) {
  address_ = address;
  ready_ = false;

  uint8_t cmd = CMD_POWER_ON;
  if (!bus_.write(address_, &cmd, 1)) return false;
  cmd = CMD_RESET;
  bus_.write(address_, &cmd, 1);
  cmd = CMD_CONT_HIGH_RES;
  if (!bus_.write(address_, &cmd, 1)) return false;

  delay(180);
  ready_ = true;
  Serial.printf("[BH1750] gotowy (0x%02X)\n", address_);
  return true;
}

bool Bh1750::read(float &lux) {
  if (!ready_) return false;
  uint8_t d[2];
  if (!bus_.read(address_, d, sizeof(d))) return false;
  uint16_t raw = static_cast<uint16_t>((d[0] << 8) | d[1]);
  lux = raw / COUNTS_PER_LUX;
  return true;
}

bool Bh1750::isReady() const {
  return ready_;
}