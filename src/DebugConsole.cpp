#include "DebugConsole.h"

DebugConsole::DebugConsole(BtAudio &bt, HwManager &hw) : bt_(bt), hw_(hw) {}

void DebugConsole::begin() {
  Serial.printf("[BT] MAC: %s\n", bt_.macAddress().c_str());
  printHelp();
}

void DebugConsole::update() {
  uint32_t version = bt_.trackVersion();
  if (version != lastTrackVersion_) {
    lastTrackVersion_ = version;
    printTrack();
  }

  if (!Serial.available()) return;
  char c = Serial.read();
  switch (c) {
    case 'p': bt_.togglePlay(); break;
    case 'n': bt_.next(); break;
    case 'b': bt_.previous(); break;
    case 't': printTrack(); break;
    case 'i': hw_.printI2cReport(); break;
    case 'h': printHelp(); break;
    default: break;
  }
}

void DebugConsole::printHelp() {
  Serial.println("Komendy: p=play/pauza n=nastepny b=poprzedni t=utwor i=skan I2C h=pomoc");
}

void DebugConsole::printTrack() {
  auto info = bt_.track();
  Serial.printf("[BT] %s - %s (%s)\n", info.artist.c_str(), info.title.c_str(), info.album.c_str());
}