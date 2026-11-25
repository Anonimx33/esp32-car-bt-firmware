#include "Ui.h"

Ui::Ui(BtAudio &bt, HwManager &hw, const char *deviceName)
    : bt_(bt), hw_(hw), deviceName_(deviceName) {}

void Ui::begin() {
  refreshMusic();
}

void Ui::update() {
  bool connected = bt_.isConnected();
  bool playing = bt_.isPlaying();
  uint32_t version = bt_.trackVersion();

  if (connected != lastConnected_ || playing != lastPlaying_ || version != lastTrackVersion_) {
    refreshMusic();
  }
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