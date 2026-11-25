#include "hw_lib/Display.h"

namespace {
constexpr int16_t WIDTH = 128;
constexpr int16_t SCROLL_GAP = 24;
constexpr uint32_t SCROLL_STEP_MS = 40;
constexpr uint32_t SCROLL_HOLD_MS = 1500;

constexpr int16_t G_CENTER_X = 32;
constexpr int16_t G_CENTER_Y = 32;
constexpr int16_t G_RADIUS = 30;
constexpr float G_FULL_SCALE = 1.0f;
}

bool Display::ScrollLine::scrolls() const {
  return width > WIDTH;
}

bool Display::begin(uint32_t busClock) {
  u8g2_.setBusClock(busClock);
  ready_ = u8g2_.begin();
  if (!ready_) return false;
  setContrast(200);

  title_.font = u8g2_font_7x13_te;
  title_.top = 16;
  title_.height = 16;
  title_.baseline = 28;

  artist_.font = u8g2_font_6x12_te;
  artist_.top = 32;
  artist_.height = 16;
  artist_.baseline = 44;

  album_.font = u8g2_font_6x12_te;
  album_.top = 48;
  album_.height = 16;
  album_.baseline = 60;

  redrawAll();
  return true;
}

bool Display::isReady() const {
  return ready_;
}

void Display::setContrast(uint8_t value) {
  if (!ready_ || value == contrast_) return;
  contrast_ = value;
  u8g2_.setContrast(value);
}

void Display::setScreen(Screen screen) {
  if (screen == screen_) return;
  screen_ = screen;
  if (ready_) redrawAll();
}

Display::Screen Display::screen() const {
  return screen_;
}

void Display::showMusic(const MusicView &view) {
  music_ = view;
  if (!ready_) return;
  if (music_.connected) {
    setLine(title_, music_.title.length() ? music_.title : String("Brak tytulu"));
    setLine(artist_, music_.artist);
    setLine(album_, music_.album);
  } else {
    setLine(title_, music_.deviceName);
    setLine(artist_, "Czekam na telefon");
    setLine(album_, "");
  }
  if (screen_ == Screen::Music) redrawAll();
}

void Display::showEnvironment(const EnvironmentView &view) {
  environment_ = view;
  if (ready_ && screen_ == Screen::Environment) redrawAll();
}

void Display::showGMeter(const GMeterView &view) {
  gmeter_ = view;
  if (ready_ && screen_ == Screen::GMeter) redrawAll();
}

void Display::setStatusText(const String &text) {
  if (text == statusText_) return;
  statusText_ = text;
  if (!ready_ || screen_ == Screen::GMeter) return;
  u8g2_.setDrawColor(0);
  u8g2_.drawBox(0, 0, WIDTH, 16);
  u8g2_.setDrawColor(1);
  drawStatusBar();
  u8g2_.updateDisplayArea(0, 0, WIDTH / 8, 2);
}

void Display::update() {
  if (!ready_ || screen_ != Screen::Music) return;
  uint32_t now = millis();
  if (stepLine(title_, now)) pushLine(title_);
  if (stepLine(artist_, now)) pushLine(artist_);
  if (stepLine(album_, now)) pushLine(album_);
}

void Display::setLine(ScrollLine &line, const String &text) {
  if (line.text == text && line.font) return;
  line.text = text;
  u8g2_.setFont(line.font);
  line.width = u8g2_.getUTF8Width(text.c_str());
  line.offset = 0;
  line.lastStep = millis();
  line.holdUntil = line.lastStep + SCROLL_HOLD_MS;
}

void Display::drawLine(ScrollLine &line) {
  u8g2_.setDrawColor(0);
  u8g2_.drawBox(0, line.top, WIDTH, line.height);
  u8g2_.setDrawColor(1);
  u8g2_.setClipWindow(0, line.top, WIDTH, line.top + line.height);
  u8g2_.setFont(line.font);
  if (line.scrolls()) {
    int16_t x = -line.offset;
    u8g2_.drawUTF8(x, line.baseline, line.text.c_str());
    u8g2_.drawUTF8(x + line.width + SCROLL_GAP, line.baseline, line.text.c_str());
  } else {
    u8g2_.drawUTF8((WIDTH - line.width) / 2, line.baseline, line.text.c_str());
  }
  u8g2_.setMaxClipWindow();
}

bool Display::stepLine(ScrollLine &line, uint32_t now) {
  if (!line.scrolls()) return false;
  if (now < line.holdUntil) return false;
  if (now - line.lastStep < SCROLL_STEP_MS) return false;
  line.lastStep = now;
  line.offset++;
  if (line.offset >= line.width + SCROLL_GAP) {
    line.offset = 0;
    line.holdUntil = now + SCROLL_HOLD_MS;
  }
  return true;
}

void Display::pushLine(ScrollLine &line) {
  drawLine(line);
  u8g2_.updateDisplayArea(0, line.top / 8, WIDTH / 8, line.height / 8);
}

void Display::drawCentered(const uint8_t *font, int16_t baseline, const char *text) {
  u8g2_.setFont(font);
  int16_t w = u8g2_.getUTF8Width(text);
  u8g2_.drawUTF8((WIDTH - w) / 2, baseline, text);
}

void Display::drawStatusBar() {
  u8g2_.setFont(u8g2_font_6x12_te);
  if (music_.connected) {
    u8g2_.drawStr(0, 10, "BT");
    if (music_.playing) {
      u8g2_.drawTriangle(118, 2, 118, 11, 126, 6);
    } else {
      u8g2_.drawBox(117, 2, 3, 9);
      u8g2_.drawBox(123, 2, 3, 9);
    }
  } else {
    u8g2_.drawStr(0, 10, "BT --");
  }
  if (statusText_.length()) {
    int16_t w = u8g2_.getUTF8Width(statusText_.c_str());
    u8g2_.drawUTF8((WIDTH - w) / 2, 10, statusText_.c_str());
  }
  u8g2_.drawHLine(0, 13, WIDTH);
}

void Display::redrawAll() {
  u8g2_.clearBuffer();
  switch (screen_) {
    case Screen::Music: drawMusicScreen(); break;
    case Screen::Environment: drawEnvironmentScreen(); break;
    case Screen::GMeter: drawGMeterScreen(); break;
  }
  u8g2_.sendBuffer();
}

void Display::drawMusicScreen() {
  drawStatusBar();
  drawLine(title_);
  drawLine(artist_);
  drawLine(album_);
}

void Display::drawEnvironmentScreen() {
  drawStatusBar();
  char buf[32];

  if (!environment_.valid) {
    drawCentered(u8g2_font_6x12_te, 38, "Brak danych BME280");
  } else {
    snprintf(buf, sizeof(buf), "%.1f\xC2\xB0" "C", environment_.temperatureC);
    drawCentered(u8g2_font_helvB18_te, 38, buf);

    if (environment_.hasHumidity) {
      snprintf(buf, sizeof(buf), "%.0f%%  %.0f hPa", environment_.humidity, environment_.pressureHpa);
    } else {
      snprintf(buf, sizeof(buf), "%.0f hPa", environment_.pressureHpa);
    }
    drawCentered(u8g2_font_6x12_te, 51, buf);
  }

  if (environment_.hasLux) {
    snprintf(buf, sizeof(buf), "\xC5\x9Awiat\xC5\x82o %.0f lx", environment_.lux);
    drawCentered(u8g2_font_6x12_te, 63, buf);
  }
}

void Display::drawGMeterScreen() {
  u8g2_.drawCircle(G_CENTER_X, G_CENTER_Y, G_RADIUS);
  u8g2_.drawCircle(G_CENTER_X, G_CENTER_Y, G_RADIUS / 2);
  u8g2_.drawHLine(G_CENTER_X - G_RADIUS, G_CENTER_Y, 2 * G_RADIUS + 1);
  u8g2_.drawVLine(G_CENTER_X, G_CENTER_Y - G_RADIUS, 2 * G_RADIUS + 1);

  u8g2_.setFont(u8g2_font_6x12_te);
  if (!gmeter_.calibrated) {
    u8g2_.drawUTF8(68, 28, "Kalibracja");
    u8g2_.drawUTF8(68, 42, "nie ruszaj");
    return;
  }

  float scale = G_RADIUS / G_FULL_SCALE;
  for (int i = 1; i < gmeter_.trailCount; ++i) {
    int16_t x = G_CENTER_X - static_cast<int16_t>(gmeter_.trailLateral[i] * scale);
    int16_t y = G_CENTER_Y + static_cast<int16_t>(gmeter_.trailLongitudinal[i] * scale);
    u8g2_.drawPixel(x, y);
  }
  int16_t x = G_CENTER_X - static_cast<int16_t>(gmeter_.lateral * scale);
  int16_t y = G_CENTER_Y + static_cast<int16_t>(gmeter_.longitudinal * scale);
  u8g2_.drawDisc(x, y, 3);

  char buf[24];
  snprintf(buf, sizeof(buf), "%.2fg", gmeter_.total);
  u8g2_.setFont(u8g2_font_helvB12_te);
  u8g2_.drawStr(68, 14, buf);

  u8g2_.setFont(u8g2_font_6x12_te);
  snprintf(buf, sizeof(buf), "Ham %.2f", gmeter_.peakBrake);
  u8g2_.drawStr(68, 28, buf);
  snprintf(buf, sizeof(buf), "Prz %.2f", gmeter_.peakAccel);
  u8g2_.drawStr(68, 40, buf);
  snprintf(buf, sizeof(buf), "L %.2f", gmeter_.peakLeft);
  u8g2_.drawStr(68, 52, buf);
  snprintf(buf, sizeof(buf), "P %.2f", gmeter_.peakRight);
  u8g2_.drawStr(68, 64, buf);
}