#pragma once

#include <Arduino.h>
#include <U8g2lib.h>

class Display {
public:
  struct MusicView {
    bool connected = false;
    bool playing = false;
    String title;
    String artist;
    String album;
    String deviceName;
  };

  bool begin(uint32_t busClock);
  void setContrast(uint8_t value);
  void showMusic(const MusicView &view);
  void update();
  bool isReady() const;

private:
  struct ScrollLine {
    String text;
    const uint8_t *font = nullptr;
    int16_t top = 0;
    int16_t height = 0;
    int16_t baseline = 0;
    int16_t width = 0;
    int16_t offset = 0;
    uint32_t lastStep = 0;
    uint32_t holdUntil = 0;

    bool scrolls() const;
  };

  void setLine(ScrollLine &line, const String &text);
  void drawLine(ScrollLine &line);
  bool stepLine(ScrollLine &line, uint32_t now);
  void pushLine(const ScrollLine &line);
  void drawStatusBar();
  void drawAlbum();
  void redrawAll();

  U8G2_SSD1309_128X64_NONAME0_F_HW_I2C u8g2_{U8G2_R0};
  bool ready_ = false;
  MusicView view_;
  ScrollLine title_;
  ScrollLine artist_;
};