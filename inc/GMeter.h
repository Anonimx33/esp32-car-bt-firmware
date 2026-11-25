#pragma once

#include <Arduino.h>
#include "hw_lib/Mpu6050.h"

class GMeter {
public:
  static constexpr int TRAIL_LENGTH = 16;

  struct State {
    bool calibrated = false;
    float lateral = 0.0f;
    float longitudinal = 0.0f;
    float total = 0.0f;
    float peakBrake = 0.0f;
    float peakAccel = 0.0f;
    float peakLeft = 0.0f;
    float peakRight = 0.0f;
    float trailLateral[TRAIL_LENGTH] = {};
    float trailLongitudinal[TRAIL_LENGTH] = {};
    int trailCount = 0;
  };

  void recalibrate();
  void resetPeaks();
  void addSample(const Mpu6050::Sample &sample, uint32_t now);
  const State &state() const;

private:
  struct Vec3 {
    float x, y, z;
  };

  static Vec3 normalize(const Vec3 &v);
  static float dot(const Vec3 &a, const Vec3 &b);
  static Vec3 cross(const Vec3 &a, const Vec3 &b);
  void buildBasis();
  void trackGravity(const Vec3 &a, const Mpu6050::Sample &s);
  void pushTrail(uint32_t now);

  State state_;
  Vec3 up_{0, 0, 1};
  Vec3 forward_{1, 0, 0};
  Vec3 right_{0, -1, 0};
  float gravityMagnitude_ = 1.0f;

  Vec3 calibSum_{0, 0, 0};
  int calibCount_ = 0;
  uint32_t lastTrail_ = 0;
};