#include "Ui.h"
#include <math.h>

namespace {
constexpr uint32_t STATUS_PERIOD_MS = 1000;
constexpr uint32_t ENVIRONMENT_PERIOD_MS = 1000;
constexpr uint32_t GMETER_PERIOD_MS = 100;
constexpr uint32_t BRIGHTNESS_PERIOD_MS = 500;
constexpr float LUX_SMOOTHING = 0.2f;
constexpr float LUX_FULL_BRIGHTNESS = 1000.0f;
constexpr uint8_t CONTRAST_MIN = 10;
constexpr uint8_t CONTRAST_MAX = 255;
constexpr uint8_t CONTRAST_STEP = 8;

constexpr Display::Screen SCREENS[] = {
    Display::Screen::Music,
    Display::Screen::Environment,
    Display::Screen::GMeter,
};
constexpr int SCREEN_COUNT = sizeof(SCREENS) / sizeof(SCREENS[0]);
}

Ui::Ui(BtAudio &bt, HwManager &hw, const char *deviceName)
    : bt_(bt), hw_(hw), deviceName_(deviceName) {}

void Ui::begin() {
  refreshMusic();
}

GMeter &Ui::gmeter() {
  return gmeter_;
}

void Ui::update() {
  uint32_t now = millis();

  handleInput();
  feedGMeter(now);

  if (bt_.isConnected() != lastConnected_ || bt_.isPlaying() != lastPlaying_ ||
      bt_.trackVersion() != lastTrackVersion_) {
    refreshMusic();
  }

  if (now - lastStatusUpdate_ >= STATUS_PERIOD_MS) {
    lastStatusUpdate_ = now;
    refreshStatus();
  }

  Display::Screen screen = hw_.display().screen();
  if (screen == Display::Screen::Environment && now - lastEnvironmentUpdate_ >= ENVIRONMENT_PERIOD_MS) {
    lastEnvironmentUpdate_ = now;
    refreshEnvironment();
  }
  if (screen == Display::Screen::GMeter && now - lastGMeterUpdate_ >= GMETER_PERIOD_MS) {
    lastGMeterUpdate_ = now;
    refreshGMeter();
  }

  if (now - lastBrightnessUpdate_ >= BRIGHTNESS_PERIOD_MS) {
    lastBrightnessUpdate_ = now;
    refreshBrightness();
  }
}

void Ui::handleInput() {
  int steps = hw_.encoder().takeSteps();
  if (steps != 0) changeScreen(steps);
  handleButton(hw_.encoder().takeButtonEvent());
}

void Ui::changeScreen(int steps) {
  Display::Screen current = hw_.display().screen();
  int index = 0;
  for (int i = 0; i < SCREEN_COUNT; ++i) {
    if (SCREENS[i] == current) index = i;
  }
  index = ((index + steps) % SCREEN_COUNT + SCREEN_COUNT) % SCREEN_COUNT;
  hw_.display().setScreen(SCREENS[index]);

  if (SCREENS[index] == Display::Screen::Environment) refreshEnvironment();
  if (SCREENS[index] == Display::Screen::GMeter) refreshGMeter();
}

void Ui::handleButton(Encoder::ButtonEvent event) {
  if (event == Encoder::ButtonEvent::None) return;
  Display::Screen screen = hw_.display().screen();

  if (screen == Display::Screen::Music) {
    if (event == Encoder::ButtonEvent::Short) bt_.togglePlay();
    else bt_.next();
  } else if (screen == Display::Screen::GMeter) {
    if (event == Encoder::ButtonEvent::Short) gmeter_.resetPeaks();
    else gmeter_.recalibrate();
    refreshGMeter();
  }
}

void Ui::feedGMeter(uint32_t now) {
  if (!hw_.hasImu()) return;
  uint32_t sequence = hw_.imuSequence();
  if (sequence == lastImuSequence_) return;
  lastImuSequence_ = sequence;
  gmeter_.addSample(hw_.imuSample(), now);
}

void Ui::refreshMusic() {
  lastConnected_ = bt_.isConnected();
  lastPlaying_ = bt_.isPlaying();
  lastTrackVersion_ = bt_.trackVersion();

  auto info = bt_.track();
  Display::MusicView view;
  view.connected = lastConnected_;
  view.playing = lastPlaying_;
  view.title = info.title;
  view.artist = info.artist;
  view.album = info.album;
  view.deviceName = deviceName_;
  hw_.display().showMusic(view);
}

void Ui::refreshStatus() {
  const auto &env = hw_.environment();
  if (!env.valid) return;
  char buf[16];
  snprintf(buf, sizeof(buf), "%.1f\xC2\xB0" "C", env.temperatureC);
  hw_.display().setStatusText(buf);
}

void Ui::refreshEnvironment() {
  const auto &env = hw_.environment();
  Display::EnvironmentView view;
  view.valid = env.valid;
  view.hasHumidity = hw_.hasHumidity();
  view.temperatureC = env.temperatureC;
  view.humidity = env.humidity;
  view.pressureHpa = env.pressureHpa;
  view.hasLux = hw_.hasLight();
  view.lux = hw_.lux();
  hw_.display().showEnvironment(view);
}

void Ui::refreshGMeter() {
  const auto &s = gmeter_.state();
  Display::GMeterView view;
  view.calibrated = s.calibrated;
  view.lateral = s.lateral;
  view.longitudinal = s.longitudinal;
  view.total = s.total;
  view.peakBrake = s.peakBrake;
  view.peakAccel = s.peakAccel;
  view.peakLeft = s.peakLeft;
  view.peakRight = s.peakRight;
  view.trailCount = s.trailCount;
  for (int i = 0; i < GMeter::TRAIL_LENGTH && i < Display::TRAIL_LENGTH; ++i) {
    view.trailLateral[i] = s.trailLateral[i];
    view.trailLongitudinal[i] = s.trailLongitudinal[i];
  }
  hw_.display().showGMeter(view);
}

void Ui::refreshBrightness() {
  if (!hw_.hasLight()) return;
  float lux = hw_.lux();
  if (smoothedLux_ < 0.0f) smoothedLux_ = lux;
  smoothedLux_ += LUX_SMOOTHING * (lux - smoothedLux_);

  float level = log10f(smoothedLux_ + 1.0f) / log10f(LUX_FULL_BRIGHTNESS + 1.0f);
  if (level > 1.0f) level = 1.0f;
  if (level < 0.0f) level = 0.0f;

  uint8_t contrast = static_cast<uint8_t>(CONTRAST_MIN + level * (CONTRAST_MAX - CONTRAST_MIN));
  contrast = (contrast / CONTRAST_STEP) * CONTRAST_STEP;
  if (contrast < CONTRAST_MIN) contrast = CONTRAST_MIN;
  hw_.display().setContrast(contrast);
}