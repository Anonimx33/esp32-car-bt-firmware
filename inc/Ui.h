#pragma once

#include <Arduino.h>
#include "BtAudio.h"
#include "hw_lib/HwManager.h"

class Ui {
public:
  Ui(BtAudio &bt, HwManager &hw, const char *deviceName);

  void begin();
  void update();

private:
  void refreshMusic();

  BtAudio &bt_;
  HwManager &hw_;
  const char *deviceName_;
  uint32_t lastTrackVersion_ = 0;
  bool lastConnected_ = false;
  bool lastPlaying_ = false;
};