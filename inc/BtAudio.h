#pragma once

#include <Arduino.h>
#include <mutex>
#include "AudioTools.h"
#include "BluetoothA2DPSink.h"

class BtAudio {
public:
  struct TrackInfo {
    String title;
    String artist;
    String album;
  };

  explicit BtAudio(const char *deviceName);

  void begin(uint8_t pinBck, uint8_t pinWs, uint8_t pinData);

  void play();
  void pause();
  void togglePlay();
  void next();
  void previous();

  bool isConnected() const;
  bool isPlaying() const;
  uint32_t trackVersion() const;
  TrackInfo track();
  String macAddress() const;

private:
  static void onMetadata(uint8_t id, const uint8_t *text);
  static void onConnection(esp_a2d_connection_state_t state, void *obj);
  static void onPlayStatus(esp_avrc_playback_stat_t status);

  static BtAudio *instance_;

  const char *deviceName_;
  I2SStream i2s_;
  BluetoothA2DPSink *sink_ = nullptr;

  std::mutex trackMutex_;
  TrackInfo track_;
  volatile uint32_t trackVersion_ = 0;
  volatile bool connected_ = false;
  volatile bool playing_ = false;
};