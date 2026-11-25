#include "BtAudio.h"
#include "esp_mac.h"

BtAudio *BtAudio::instance_ = nullptr;

BtAudio::BtAudio(const char *deviceName) : deviceName_(deviceName) {
  instance_ = this;
}

void BtAudio::begin(uint8_t pinBck, uint8_t pinWs, uint8_t pinData) {
  auto cfg = i2s_.defaultConfig();
  cfg.pin_bck = pinBck;
  cfg.pin_ws = pinWs;
  cfg.pin_data = pinData;
  i2s_.begin(cfg);

  sink_ = new BluetoothA2DPSink(i2s_);
  sink_->set_avrc_metadata_attribute_mask(ESP_AVRC_MD_ATTR_TITLE | ESP_AVRC_MD_ATTR_ARTIST | ESP_AVRC_MD_ATTR_ALBUM);
  sink_->set_avrc_metadata_callback(onMetadata);
  sink_->set_avrc_rn_playstatus_callback(onPlayStatus);
  sink_->set_on_connection_state_changed(onConnection, this);
  sink_->set_auto_reconnect(true);
  sink_->start(deviceName_);
}

void BtAudio::play() {
  if (sink_ && connected_) sink_->play();
}

void BtAudio::pause() {
  if (sink_ && connected_) sink_->pause();
}

void BtAudio::togglePlay() {
  if (playing_) pause();
  else play();
}

void BtAudio::next() {
  if (sink_ && connected_) sink_->next();
}

void BtAudio::previous() {
  if (sink_ && connected_) sink_->previous();
}

bool BtAudio::isConnected() const {
  return connected_;
}

bool BtAudio::isPlaying() const {
  return playing_;
}

uint32_t BtAudio::trackVersion() const {
  return trackVersion_;
}

BtAudio::TrackInfo BtAudio::track() {
  std::lock_guard<std::mutex> lock(trackMutex_);
  return track_;
}

String BtAudio::macAddress() const {
  uint8_t mac[6];
  esp_read_mac(mac, ESP_MAC_BT);
  char buf[18];
  snprintf(buf, sizeof(buf), "%02X:%02X:%02X:%02X:%02X:%02X",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  return String(buf);
}

void BtAudio::onMetadata(uint8_t id, const uint8_t *text) {
  if (!instance_) return;
  std::lock_guard<std::mutex> lock(instance_->trackMutex_);
  String value = reinterpret_cast<const char *>(text);
  switch (id) {
    case ESP_AVRC_MD_ATTR_TITLE: instance_->track_.title = value; break;
    case ESP_AVRC_MD_ATTR_ARTIST: instance_->track_.artist = value; break;
    case ESP_AVRC_MD_ATTR_ALBUM: instance_->track_.album = value; break;
    default: return;
  }
  instance_->trackVersion_ = instance_->trackVersion_ + 1;
}

void BtAudio::onConnection(esp_a2d_connection_state_t state, void *obj) {
  auto *self = static_cast<BtAudio *>(obj);
  self->connected_ = (state == ESP_A2D_CONNECTION_STATE_CONNECTED);
  if (!self->connected_) self->playing_ = false;
  Serial.printf("[BT] %s\n", self->sink_->to_str(state));
}

void BtAudio::onPlayStatus(esp_avrc_playback_stat_t status) {
  if (!instance_) return;
  instance_->playing_ = (status == ESP_AVRC_PLAYBACK_PLAYING);
}