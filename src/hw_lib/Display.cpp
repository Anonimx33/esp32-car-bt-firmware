#include "hw_lib/Display.h"

namespace {
constexpr int16_t WIDTH = 128;
constexpr int16_t SCROLL_GAP = 24;
constexpr uint32_t SCROLL_STEP_MS = 40;
constexpr uint32_t SCROLL_HOLD_MS = 1500;
}

bool Display::ScrollLine::scrolls() const {
  return width > WIDTH;
}

bool Display::begin(uint32_t busClock) {
  u8g2_.setBusClock(busClock);
  ready_ = u8g2_.begin();
  if (!ready_) return false;
  u8g2_.setContrast(200);

  title_.font = u8g2_font_7x13_te;
  title_.top = 16;
  title_.height = 16;
  title_.baseline = 28;

  artist_.font = u8g2_font_6x12_te;
  artist_.top = 32;
  artist_.height = 16;
  artist_.baseline = 44;

  redrawAll();
  return true;
}

void Display::setContrast(uint8_t value) {
  if (ready_) u8g2_.setContrast(value);
}

bool Display::isReady() const {
  return ready_;
}

void Display::showMusic(const MusicView &view) {
  view_ = view;
  if (!ready_) return;
  if (view_.connected) {
    setLine(title_, view_.title.length() ? view_.title : String("Brak tytulu"));
    setLine(artist_, view_.artist);
  } else {
    setLine(title_, view_.deviceName);
    setLine(artist_, "Czekam na telefon");
  }
  redrawAll();
}

void Display::update() {
  if (!ready_) return;
  uint32_t now = millis();
  if (stepLine(title_, now)) pushLine(title_);
  if (stepLine(artist_, now)) pushLine(artist_);
}

void Display::setLine(ScrollLine &line, const String &text) {
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
    int16_t x = (WIDTH - line.width) / 2;
    u8g2_.drawUTF8(x, line.baseline, line.text.c_str());
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

void Display::pushLine(const ScrollLine &line) {
  drawLine(const_cast<ScrollLine &>(line));
  u8g2_.updateDisplayArea(0, line.top / 8, WIDTH / 8, line.height / 8);
}

void Display::drawStatusBar() {
  u8g2_.setFont(u8g2_font_6x12_te);
  if (view_.connected) {
    u8g2_.drawStr(0, 10, "BT");
    if (view_.playing) {
      u8g2_.drawTriangle(118, 2, 118, 11, 126, 6);
    } else {
      u8g2_.drawBox(117, 2, 3, 9);
      u8g2_.drawBox(123, 2, 3, 9);
    }
  } else {
    u8g2_.drawStr(0, 10, "BT --");
  }
  u8g2_.drawHLine(0, 13, WIDTH);
}

void Display::drawAlbum() {
  if (!view_.connected || view_.album.length() == 0) return;
  u8g2_.setFont(u8g2_font_6x12_te);
  u8g2_.setClipWindow(0, 50, WIDTH, 64);
  int16_t w = u8g2_.getUTF8Width(view_.album.c_str());
  int16_t x = w < WIDTH ? (WIDTH - w) / 2 : 0;
  u8g2_.drawUTF8(x, 62, view_.album.c_str());
  u8g2_.setMaxClipWindow();
}

void Display::redrawAll() {
  u8g2_.clearBuffer();
  drawStatusBar();
  drawLine(title_);
  drawLine(artist_);
  drawAlbum();
  u8g2_.sendBuffer();
}