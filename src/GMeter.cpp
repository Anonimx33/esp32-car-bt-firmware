#include "GMeter.h"
#include <math.h>

namespace {
constexpr int CALIBRATION_SAMPLES = 100;
constexpr float SMOOTHING = 0.15f;
constexpr float GRAVITY_TRACK_RATE = 0.002f;
constexpr float STILL_ACCEL_TOLERANCE = 0.05f;
constexpr float STILL_GYRO_DPS = 3.0f;
constexpr uint32_t TRAIL_PERIOD_MS = 100;
}

GMeter::Vec3 GMeter::normalize(const Vec3 &v) {
  float n = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
  if (n < 1e-6f) return {0, 0, 1};
  return {v.x / n, v.y / n, v.z / n};
}

float GMeter::dot(const Vec3 &a, const Vec3 &b) {
  return a.x * b.x + a.y * b.y + a.z * b.z;
}

GMeter::Vec3 GMeter::cross(const Vec3 &a, const Vec3 &b) {
  return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}

void GMeter::recalibrate() {
  calibSum_ = {0, 0, 0};
  calibCount_ = 0;
  state_ = State();
}

void GMeter::resetPeaks() {
  state_.peakBrake = state_.peakAccel = state_.peakLeft = state_.peakRight = 0.0f;
  state_.trailCount = 0;
}

void GMeter::buildBasis() {
  Vec3 sensorX{1, 0, 0};
  Vec3 f{sensorX.x - dot(sensorX, up_) * up_.x,
         sensorX.y - dot(sensorX, up_) * up_.y,
         sensorX.z - dot(sensorX, up_) * up_.z};
  if (sqrtf(dot(f, f)) < 0.3f) {
    Vec3 sensorY{0, 1, 0};
    f = {sensorY.x - dot(sensorY, up_) * up_.x,
         sensorY.y - dot(sensorY, up_) * up_.y,
         sensorY.z - dot(sensorY, up_) * up_.z};
  }
  forward_ = normalize(f);
  right_ = cross(forward_, up_);
}

void GMeter::trackGravity(const Vec3 &a, const Mpu6050::Sample &s) {
  float magnitude = sqrtf(dot(a, a));
  float gyro = sqrtf(s.gx * s.gx + s.gy * s.gy + s.gz * s.gz);
  if (fabsf(magnitude - gravityMagnitude_) > STILL_ACCEL_TOLERANCE) return;
  if (gyro > STILL_GYRO_DPS) return;
  Vec3 unit = normalize(a);
  up_ = normalize({up_.x + GRAVITY_TRACK_RATE * (unit.x - up_.x),
                   up_.y + GRAVITY_TRACK_RATE * (unit.y - up_.y),
                   up_.z + GRAVITY_TRACK_RATE * (unit.z - up_.z)});
  buildBasis();
}

void GMeter::addSample(const Mpu6050::Sample &sample, uint32_t now) {
  Vec3 a{sample.ax, sample.ay, sample.az};

  if (!state_.calibrated) {
    calibSum_.x += a.x;
    calibSum_.y += a.y;
    calibSum_.z += a.z;
    if (++calibCount_ >= CALIBRATION_SAMPLES) {
      Vec3 mean{calibSum_.x / calibCount_, calibSum_.y / calibCount_, calibSum_.z / calibCount_};
      gravityMagnitude_ = sqrtf(dot(mean, mean));
      up_ = normalize(mean);
      buildBasis();
      state_.calibrated = true;
      Serial.printf("[G] Kalibracja: |g|=%.3f\n", gravityMagnitude_);
    }
    return;
  }

  trackGravity(a, sample);

  Vec3 dynamic{a.x - up_.x * gravityMagnitude_,
               a.y - up_.y * gravityMagnitude_,
               a.z - up_.z * gravityMagnitude_};
  float lon = dot(dynamic, forward_);
  float lat = dot(dynamic, right_);

  state_.longitudinal += SMOOTHING * (lon - state_.longitudinal);
  state_.lateral += SMOOTHING * (lat - state_.lateral);
  state_.total = sqrtf(state_.longitudinal * state_.longitudinal + state_.lateral * state_.lateral);

  if (-state_.longitudinal > state_.peakBrake) state_.peakBrake = -state_.longitudinal;
  if (state_.longitudinal > state_.peakAccel) state_.peakAccel = state_.longitudinal;
  if (-state_.lateral > state_.peakLeft) state_.peakLeft = -state_.lateral;
  if (state_.lateral > state_.peakRight) state_.peakRight = state_.lateral;

  pushTrail(now);
}

void GMeter::pushTrail(uint32_t now) {
  if (now - lastTrail_ < TRAIL_PERIOD_MS) return;
  lastTrail_ = now;
  for (int i = TRAIL_LENGTH - 1; i > 0; --i) {
    state_.trailLateral[i] = state_.trailLateral[i - 1];
    state_.trailLongitudinal[i] = state_.trailLongitudinal[i - 1];
  }
  state_.trailLateral[0] = state_.lateral;
  state_.trailLongitudinal[0] = state_.longitudinal;
  if (state_.trailCount < TRAIL_LENGTH) state_.trailCount++;
}

const GMeter::State &GMeter::state() const {
  return state_;
}