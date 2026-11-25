#pragma once

#include <Arduino.h>
#include "BtAudio.h"
#include "GMeter.h"
#include "hw_lib/HwManager.h"

class Ui {
public:
  Ui(BtAudio &bt, HwManager &hw, const char *deviceName);

  void begin();
  void update();

  GMeter &gmeter();

private:
  void handleInput();
  void handleButton(Encoder::ButtonEvent event);
  void changeScreen(int steps);

  void feedGMeter(uint32_t now);
  void refreshMusic();
  void refreshStatus();
  void refreshEnvironment();
  void refreshGMeter();
  void refreshBrightness();

  BtAudio &bt_;
  HwManager &hw_;
  const char *deviceName_;
  GMeter gmeter_;

  uint32_t lastTrackVersion_ = 0;
  bool lastConnected_ = false;
  bool lastPlaying_ = false;
  uint32_t lastImuSequence_ = 0;

  uint32_t lastStatusUpdate_ = 0;
  uint32_t lastEnvironmentUpdate_ = 0;
  uint32_t lastGMeterUpdate_ = 0;
  uint32_t lastBrightnessUpdate_ = 0;
  float smoothedLux_ = -1.0f;
};