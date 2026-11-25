#include "hw_lib/Encoder.h"

namespace {
constexpr int8_t QUADRATURE_TABLE[16] = {0, -1, 1, 0, 1, 0, 0, -1, -1, 0, 0, 1, 0, 1, -1, 0};
constexpr int32_t TRANSITIONS_PER_DETENT = 4;
constexpr bool INVERT_DIRECTION = false;
constexpr uint32_t DEBOUNCE_MS = 30;
constexpr uint32_t LONG_PRESS_MS = 700;
}

bool Encoder::begin(uint8_t pinA, uint8_t pinB, uint8_t pinSw) {
  pinA_ = pinA;
  pinB_ = pinB;
  pinSw_ = pinSw;

  pinMode(pinA_, INPUT_PULLUP);
  pinMode(pinB_, INPUT_PULLUP);
  pinMode(pinSw_, INPUT_PULLUP);

  state_ = (digitalRead(pinA_) << 1) | digitalRead(pinB_);
  attachInterruptArg(digitalPinToInterrupt(pinA_), onEdge, this, CHANGE);
  attachInterruptArg(digitalPinToInterrupt(pinB_), onEdge, this, CHANGE);

  ready_ = true;
  Serial.println("[ENC] gotowy");
  return true;
}

void IRAM_ATTR Encoder::onEdge(void *arg) {
  auto *self = static_cast<Encoder *>(arg);
  uint8_t current = (digitalRead(self->pinA_) << 1) | digitalRead(self->pinB_);
  portENTER_CRITICAL_ISR(&self->mux_);
  self->state_ = ((self->state_ << 2) | current) & 0x0F;
  self->transitions_ += QUADRATURE_TABLE[self->state_];
  portEXIT_CRITICAL_ISR(&self->mux_);
}

int Encoder::takeSteps() {
  if (!ready_) return 0;
  portENTER_CRITICAL(&mux_);
  int32_t steps = transitions_ / TRANSITIONS_PER_DETENT;
  transitions_ -= steps * TRANSITIONS_PER_DETENT;
  portEXIT_CRITICAL(&mux_);
  return INVERT_DIRECTION ? -steps : steps;
}

void Encoder::update() {
  if (!ready_) return;
  uint32_t now = millis();
  bool raw = digitalRead(pinSw_) == LOW;

  if (raw != rawPressed_) {
    rawPressed_ = raw;
    lastRawChange_ = now;
  }
  if (now - lastRawChange_ < DEBOUNCE_MS) return;

  if (rawPressed_ && !stablePressed_) {
    stablePressed_ = true;
    pressStart_ = now;
    longFired_ = false;
  } else if (!rawPressed_ && stablePressed_) {
    stablePressed_ = false;
    if (!longFired_) pendingEvent_ = ButtonEvent::Short;
  }

  if (stablePressed_ && !longFired_ && now - pressStart_ >= LONG_PRESS_MS) {
    longFired_ = true;
    pendingEvent_ = ButtonEvent::Long;
  }
}

Encoder::ButtonEvent Encoder::takeButtonEvent() {
  ButtonEvent event = pendingEvent_;
  pendingEvent_ = ButtonEvent::None;
  return event;
}