#pragma once

#include <Arduino.h>

class Encoder {
public:
  enum class ButtonEvent { None, Short, Long };

  bool begin(uint8_t pinA, uint8_t pinB, uint8_t pinSw);
  void update();
  int takeSteps();
  ButtonEvent takeButtonEvent();

private:
  static void IRAM_ATTR onEdge(void *arg);

  uint8_t pinA_ = 0;
  uint8_t pinB_ = 0;
  uint8_t pinSw_ = 0;
  bool ready_ = false;

  portMUX_TYPE mux_ = portMUX_INITIALIZER_UNLOCKED;
  volatile int32_t transitions_ = 0;
  volatile uint8_t state_ = 0;

  bool rawPressed_ = false;
  bool stablePressed_ = false;
  uint32_t lastRawChange_ = 0;
  uint32_t pressStart_ = 0;
  bool longFired_ = false;
  ButtonEvent pendingEvent_ = ButtonEvent::None;
};