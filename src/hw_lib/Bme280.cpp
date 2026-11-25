#include "hw_lib/Bme280.h"

namespace {
constexpr uint8_t REG_CALIB_TP = 0x88;
constexpr uint8_t REG_CHIP_ID = 0xD0;
constexpr uint8_t REG_RESET = 0xE0;
constexpr uint8_t REG_CALIB_H = 0xE1;
constexpr uint8_t REG_CTRL_HUM = 0xF2;
constexpr uint8_t REG_STATUS = 0xF3;
constexpr uint8_t REG_CTRL_MEAS = 0xF4;
constexpr uint8_t REG_CONFIG = 0xF5;
constexpr uint8_t REG_DATA = 0xF7;

constexpr uint8_t CHIP_ID_BME280 = 0x60;
constexpr uint8_t CHIP_ID_BMP280 = 0x58;
constexpr uint8_t RESET_COMMAND = 0xB6;

constexpr uint8_t OSRS_H_X1 = 0b001;
constexpr uint8_t OSRS_T_X2 = 0b010;
constexpr uint8_t OSRS_P_X4 = 0b011;
constexpr uint8_t MODE_NORMAL = 0b11;
constexpr uint8_t STANDBY_1000MS = 0b101;
constexpr uint8_t FILTER_X4 = 0b010;

uint16_t u16le(const uint8_t *p) { return static_cast<uint16_t>(p[0] | (p[1] << 8)); }
int16_t s16le(const uint8_t *p) { return static_cast<int16_t>(u16le(p)); }
}

Bme280::Bme280(I2cBus &bus) : bus_(bus) {}

bool Bme280::begin(uint8_t address) {
  address_ = address;
  ready_ = false;

  uint8_t id = 0;
  if (!bus_.readRegs(address_, REG_CHIP_ID, &id, 1)) return false;
  if (id == CHIP_ID_BME280) hasHumidity_ = true;
  else if (id == CHIP_ID_BMP280) hasHumidity_ = false;
  else {
    Serial.printf("[BME] Nieznany chip ID 0x%02X\n", id);
    return false;
  }

  bus_.writeReg(address_, REG_RESET, RESET_COMMAND);
  delay(5);
  uint8_t status = 0;
  for (int i = 0; i < 20; ++i) {
    bus_.readRegs(address_, REG_STATUS, &status, 1);
    if ((status & 0x01) == 0) break;
    delay(2);
  }

  if (!readCalibration()) return false;

  if (hasHumidity_) bus_.writeReg(address_, REG_CTRL_HUM, OSRS_H_X1);
  bus_.writeReg(address_, REG_CONFIG, (STANDBY_1000MS << 5) | (FILTER_X4 << 2));
  bus_.writeReg(address_, REG_CTRL_MEAS, (OSRS_T_X2 << 5) | (OSRS_P_X4 << 2) | MODE_NORMAL);

  ready_ = true;
  Serial.printf("[BME] %s gotowy (0x%02X)\n", hasHumidity_ ? "BME280" : "BMP280", address_);
  return true;
}

bool Bme280::readCalibration() {
  uint8_t tp[26];
  if (!bus_.readRegs(address_, REG_CALIB_TP, tp, sizeof(tp))) return false;
  digT1_ = u16le(&tp[0]);
  digT2_ = s16le(&tp[2]);
  digT3_ = s16le(&tp[4]);
  digP1_ = u16le(&tp[6]);
  digP2_ = s16le(&tp[8]);
  digP3_ = s16le(&tp[10]);
  digP4_ = s16le(&tp[12]);
  digP5_ = s16le(&tp[14]);
  digP6_ = s16le(&tp[16]);
  digP7_ = s16le(&tp[18]);
  digP8_ = s16le(&tp[20]);
  digP9_ = s16le(&tp[22]);
  digH1_ = tp[25];

  if (!hasHumidity_) return true;

  uint8_t h[7];
  if (!bus_.readRegs(address_, REG_CALIB_H, h, sizeof(h))) return false;
  digH2_ = s16le(&h[0]);
  digH3_ = h[2];
  digH4_ = static_cast<int16_t>((static_cast<int8_t>(h[3]) * 16) | (h[4] & 0x0F));
  digH5_ = static_cast<int16_t>((static_cast<int8_t>(h[5]) * 16) | (h[4] >> 4));
  digH6_ = static_cast<int8_t>(h[6]);
  return true;
}

bool Bme280::read(Reading &out) {
  out.valid = false;
  if (!ready_) return false;

  uint8_t d[8];
  if (!bus_.readRegs(address_, REG_DATA, d, sizeof(d))) return false;

  int32_t adcP = (static_cast<int32_t>(d[0]) << 12) | (d[1] << 4) | (d[2] >> 4);
  int32_t adcT = (static_cast<int32_t>(d[3]) << 12) | (d[4] << 4) | (d[5] >> 4);
  int32_t adcH = (static_cast<int32_t>(d[6]) << 8) | d[7];

  if (adcT == 0x80000) return false;

  out.temperatureC = compensateTemperature(adcT) / 100.0f;
  out.pressureHpa = compensatePressure(adcP) / 25600.0f;
  out.humidity = hasHumidity_ ? compensateHumidity(adcH) / 1024.0f : 0.0f;
  out.valid = true;
  return true;
}

int32_t Bme280::compensateTemperature(int32_t adc) {
  int32_t var1 = ((((adc >> 3) - (static_cast<int32_t>(digT1_) << 1))) * digT2_) >> 11;
  int32_t var2 = (((((adc >> 4) - digT1_) * ((adc >> 4) - digT1_)) >> 12) * digT3_) >> 14;
  tFine_ = var1 + var2;
  return (tFine_ * 5 + 128) >> 8;
}

uint32_t Bme280::compensatePressure(int32_t adc) const {
  int64_t var1 = static_cast<int64_t>(tFine_) - 128000;
  int64_t var2 = var1 * var1 * digP6_;
  var2 = var2 + ((var1 * digP5_) << 17);
  var2 = var2 + (static_cast<int64_t>(digP4_) << 35);
  var1 = ((var1 * var1 * digP3_) >> 8) + ((var1 * digP2_) << 12);
  var1 = (((static_cast<int64_t>(1) << 47) + var1) * digP1_) >> 33;
  if (var1 == 0) return 0;
  int64_t p = 1048576 - adc;
  p = (((p << 31) - var2) * 3125) / var1;
  var1 = (static_cast<int64_t>(digP9_) * (p >> 13) * (p >> 13)) >> 25;
  var2 = (static_cast<int64_t>(digP8_) * p) >> 19;
  p = ((p + var1 + var2) >> 8) + (static_cast<int64_t>(digP7_) << 4);
  return static_cast<uint32_t>(p);
}

uint32_t Bme280::compensateHumidity(int32_t adc) const {
  int32_t v = tFine_ - 76800;
  v = (((((adc << 14) - (static_cast<int32_t>(digH4_) << 20) - (static_cast<int32_t>(digH5_) * v)) + 16384) >> 15) *
       (((((((v * digH6_) >> 10) * (((v * static_cast<int32_t>(digH3_)) >> 11) + 32768)) >> 10) + 2097152) * digH2_ + 8192) >> 14));
  v = v - (((((v >> 15) * (v >> 15)) >> 7) * static_cast<int32_t>(digH1_)) >> 4);
  if (v < 0) v = 0;
  if (v > 419430400) v = 419430400;
  return static_cast<uint32_t>(v >> 12);
}

bool Bme280::hasHumidity() const {
  return hasHumidity_;
}

bool Bme280::isReady() const {
  return ready_;
}