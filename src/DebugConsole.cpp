#include "DebugConsole.h"

DebugConsole::DebugConsole(BtAudio &bt, HwManager &hw) : bt_(bt), hw_(hw) {}

void DebugConsole::begin() {
  Serial.printf("[BT] MAC: %s\n", bt_.macAddress().c_str());
  printHelp();
}

void DebugConsole::update() {
  uint32_t version = bt_.trackVersion();
  if (version != lastTrackVersion_) {
    lastTrackVersion_ = version;
    printTrack();
  }

  if (!Serial.available()) return;
  char c = Serial.read();
  switch (c) {
    case 'p': bt_.togglePlay(); break;
    case 'n': bt_.next(); break;
    case 'b': bt_.previous(); break;
    case 't': printTrack(); break;
    case 'i': hw_.printI2cReport(); break;
    case 's': printSensors(); break;
    case 'm': printImu(); break;
    case 'h': printHelp(); break;
    default: break;
  }
}

void DebugConsole::printHelp() {
  Serial.println("Komendy: p=play/pauza n=nastepny b=poprzedni t=utwor i=skan I2C s=czujniki m=MPU6050 h=pomoc");
}

void DebugConsole::printTrack() {
  auto info = bt_.track();
  Serial.printf("[BT] %s - %s (%s)\n", info.artist.c_str(), info.title.c_str(), info.album.c_str());
}

void DebugConsole::printSensors() {
  const auto &env = hw_.environment();
  if (env.valid) {
    Serial.printf("[ENV] %.2f C  %.1f %%RH  %.2f hPa\n", env.temperatureC, env.humidity, env.pressureHpa);
  } else {
    Serial.println("[ENV] brak odczytu BME280");
  }
  if (hw_.hasLight()) Serial.printf("[LUX] %.1f lx\n", hw_.lux());
  else Serial.println("[LUX] brak odczytu BH1750");
}

void DebugConsole::printImu() {
  if (!hw_.hasImu()) {
    Serial.println("[MPU] brak MPU6050");
    return;
  }
  const auto &s = hw_.imuSample();
  Serial.printf("[MPU] acc %.3f %.3f %.3f g  gyro %.1f %.1f %.1f dps  %.1f C\n",
                s.ax, s.ay, s.az, s.gx, s.gy, s.gz, s.temperatureC);
}