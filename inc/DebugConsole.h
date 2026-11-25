#pragma once

#include <Arduino.h>
#include "BtAudio.h"
#include "hw_lib/HwManager.h"

class DebugConsole {
public:
  DebugConsole(BtAudio &bt, HwManager &hw);

  void begin();
  void update();

private:
  void printHelp();
  void printTrack();

  BtAudio &bt_;
  HwManager &hw_;
  uint32_t lastTrackVersion_ = 0;
};