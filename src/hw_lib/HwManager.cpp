#include "hw_lib/HwManager.h"
#include "Pins.h"

namespace {
constexpr uint32_t I2C_FREQUENCY = 400000;
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
  return true;
}

void HwManager::update() {
  display_.update();
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
  for (const auto &device : devices_) {
    if (device.address == 0x77) continue;
    if (device.address == 0x76 && bmeAlt) continue;
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