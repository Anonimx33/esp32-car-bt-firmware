#include "hw_lib/HwManager.h"
#include "Pins.h"

namespace {
constexpr uint32_t I2C_FREQUENCY = 400000;
constexpr uint32_t BME_PERIOD_MS = 1000;
constexpr uint32_t LIGHT_PERIOD_MS = 500;
constexpr uint32_t IMU_PERIOD_MS = 10;
}

bool HwManager::begin() {
  if (!i2c_.begin(Pins::I2C_SDA, Pins::I2C_SCL, I2C_FREQUENCY)) {
    Serial.println("[HW] Blad inicjalizacji I2C");
    return false;
  }
  detectDevices();
  printI2cReport();

  if (isDetected(0x3C)) {
    if (display_.begin(I2C_FREQUENCY)) Serial.println("[HW] OLED gotowy");
    else Serial.println("[HW] Blad inicjalizacji OLED");
  }

  if (isDetected(0x76)) bme_.begin(0x76);
  else if (isDetected(0x77)) bme_.begin(0x77);

  if (isDetected(0x23)) light_.begin(0x23);

  if (isDetected(0x68)) imu_.begin(0x68);
  else if (isDetected(0x69)) imu_.begin(0x69);

  encoder_.begin(Pins::ENC_A, Pins::ENC_B, Pins::ENC_SW);
  return true;
}

void HwManager::update() {
  uint32_t now = millis();
  encoder_.update();
  updateSensors(now);
  display_.update();
}

void HwManager::updateSensors(uint32_t now) {
  if (imu_.isReady() && now - lastImuRead_ >= IMU_PERIOD_MS) {
    lastImuRead_ = now;
    if (imu_.read(imuSample_)) imuSequence_++;
  }

  if (bme_.isReady() && now - lastBmeRead_ >= BME_PERIOD_MS) {
    lastBmeRead_ = now;
    Bme280::Reading reading;
    if (bme_.read(reading)) environment_ = reading;
  }

  if (light_.isReady() && now - lastLightRead_ >= LIGHT_PERIOD_MS) {
    lastLightRead_ = now;
    float value = 0.0f;
    if (light_.read(value)) {
      lux_ = value;
      luxValid_ = true;
    }
  }
}

void HwManager::detectDevices() {
  for (auto &device : devices_) {
    device.present = i2c_.isPresent(device.address);
  }
}

bool HwManager::isDetected(uint8_t address) const {
  for (const auto &device : devices_) {
    if (device.address == address) return device.present;
  }
  return false;
}

void HwManager::printI2cReport() {
  Serial.println("[HW] Skan I2C:");
  auto found = i2c_.scan();
  if (found.empty()) {
    Serial.println("[HW]   brak urzadzen - sprawdz SDA/SCL i zasilanie");
  }
  for (uint8_t address : found) {
    const char *name = "nieznane";
    for (const auto &device : devices_) {
      if (device.address == address) name = device.name;
    }
    Serial.printf("[HW]   0x%02X  %s\n", address, name);
  }
  bool bmeAlt = i2c_.isPresent(0x77);
  bool imuAlt = i2c_.isPresent(0x69);
  for (const auto &device : devices_) {
    if (device.address == 0x77 || device.address == 0x69) continue;
    if (device.address == 0x76 && bmeAlt) continue;
    if (device.address == 0x68 && imuAlt) continue;
    if (!device.present) {
      Serial.printf("[HW]   BRAK: %s (0x%02X)\n", device.name, device.address);
    }
  }
}

I2cBus &HwManager::i2c() {
  return i2c_;
}

Display &HwManager::display() {
  return display_;
}

Encoder &HwManager::encoder() {
  return encoder_;
}

const Bme280::Reading &HwManager::environment() const {
  return environment_;
}

bool HwManager::hasHumidity() const {
  return bme_.hasHumidity();
}

bool HwManager::hasLight() const {
  return luxValid_;
}

float HwManager::lux() const {
  return lux_;
}

bool HwManager::hasImu() const {
  return imu_.isReady();
}

uint32_t HwManager::imuSequence() const {
  return imuSequence_;
}

const Mpu6050::Sample &HwManager::imuSample() const {
  return imuSample_;
}