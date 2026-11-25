#pragma once

#include <Arduino.h>
#include <U8g2lib.h>

class Display {
public:
  enum class Screen { Music, Environment, GMeter };

  struct MusicView {
    bool connected = false;
    bool playing = false;
    String title;
    String artist;
    String album;
    String deviceName;
  };

  struct EnvironmentView {
    bool valid = false;
    bool hasHumidity = false;
    float temperatureC = 0.0f;
    float humidity = 0.0f;
    float pressureHpa = 0.0f;
    bool hasLux = false;
    float lux = 0.0f;
  };

  static constexpr int TRAIL_LENGTH = 16;

  struct GMeterView {
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

  bool begin(uint32_t busClock);
  void setContrast(uint8_t value);
  void setScreen(Screen screen);
  Screen screen() const;

  void showMusic(const MusicView &view);
  void showEnvironment(const EnvironmentView &view);
  void showGMeter(const GMeterView &view);
  void setStatusText(const String &text);

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
  void pushLine(ScrollLine &line);
  void drawStatusBar();
  void drawCentered(const uint8_t *font, int16_t baseline, const char *text);

  void redrawAll();
  void drawMusicScreen();
  void drawEnvironmentScreen();
  void drawGMeterScreen();

  U8G2_SSD1309_128X64_NONAME0_F_HW_I2C u8g2_{U8G2_R0};
  bool ready_ = false;
  Screen screen_ = Screen::Music;
  uint8_t contrast_ = 0;
  String statusText_;

  MusicView music_;
  EnvironmentView environment_;
  GMeterView gmeter_;

  ScrollLine title_;
  ScrollLine artist_;
  ScrollLine album_;
};