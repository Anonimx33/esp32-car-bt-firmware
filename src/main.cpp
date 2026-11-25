#include <Arduino.h>
#include "Pins.h"
#include "BtAudio.h"
#include "DebugConsole.h"
#include "Ui.h"
#include "hw_lib/HwManager.h"

constexpr const char *DEVICE_NAME = "Yaris Audio";

BtAudio bt(DEVICE_NAME);
HwManager hw;
Ui ui(bt, hw, DEVICE_NAME);
DebugConsole console(bt, hw);

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("Start");

  hw.begin();
  ui.begin();
  bt.begin(Pins::I2S_BCK, Pins::I2S_WS, Pins::I2S_DATA);
  console.begin();
}

void loop() {
  hw.update();
  ui.update();
  console.update();
  delay(5);
}