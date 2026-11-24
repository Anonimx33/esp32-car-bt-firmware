#include <Arduino.h>
#include "esp_mac.h"
#include "AudioTools.h"
#include "BluetoothA2DPSink.h"

I2SStream i2s;
BluetoothA2DPSink *a2dp = nullptr;

void onMetadata(uint8_t id, const uint8_t *text) {
  switch (id) {
    case ESP_AVRC_MD_ATTR_TITLE:
      Serial.printf("Tytul: %s\n", text);
      break;
    case ESP_AVRC_MD_ATTR_ARTIST:
      Serial.printf("Wykonawca: %s\n", text);
      break;
    case ESP_AVRC_MD_ATTR_ALBUM:
      Serial.printf("Album: %s\n", text);
      break;
  }
}

void onConnection(esp_a2d_connection_state_t state, void *) {
  Serial.printf("Polaczenie: %s\n", a2dp->to_str(state));
}

void printBtMac() {
  uint8_t mac[6];
  esp_read_mac(mac, ESP_MAC_BT);
  Serial.printf("MAC Bluetooth: %02X:%02X:%02X:%02X:%02X:%02X\n",
                mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

void handleSerial() {
  if (!Serial.available()) return;
  char c = Serial.read();
  switch (c) {
    case 'p': a2dp->play(); Serial.println("> play"); break;
    case 's': a2dp->pause(); Serial.println("> pauza"); break;
    case 'n': a2dp->next(); Serial.println("> nastepny"); break;
    case 'b': a2dp->previous(); Serial.println("> poprzedni"); break;
  }
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("Start");

  auto cfg = i2s.defaultConfig();
  cfg.pin_bck = 26;
  cfg.pin_ws = 25;
  cfg.pin_data = 27;
  i2s.begin(cfg);

  a2dp = new BluetoothA2DPSink(i2s);
  a2dp->set_avrc_metadata_attribute_mask(ESP_AVRC_MD_ATTR_TITLE | ESP_AVRC_MD_ATTR_ARTIST | ESP_AVRC_MD_ATTR_ALBUM);
  a2dp->set_avrc_metadata_callback(onMetadata);
  a2dp->set_on_connection_state_changed(onConnection);
  a2dp->set_auto_reconnect(true);
  a2dp->start("Yaris Audio");

  printBtMac();
  Serial.println("Komendy: p=play s=pauza n=nastepny b=poprzedni");
}

void loop() {
  handleSerial();
  delay(10);
}