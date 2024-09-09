/**
 * @file streams-sd_mp3-i2s.ino
 * @author Phil Schatzmann
 * @brief decode MP3 file and output it on I2S
 * @version 0.1
 * @date 2021-96-25
 * 
 * @copyright Copyright (c) 2021 
 */

#include "AudioTools.h"
#include "AudioLibs/A2DPStream.h"

void callbackInit();
Stream* callbackNextStream(int offset);

#define RXD2 16
#define TXD2 17

auto &serial = Serial2;
Throttle throttle(serial);
AudioSourceCallback source(callbackNextStream,callbackInit);
A2DPStream out;
CopyDecoder decoder;
AudioPlayer player(source, out, decoder);
//StreamCopy copier(out,throttle);

void callbackInit() {
  // make sure that the directory contains only mp3 files
  //dir = SD.open("/TomWaits");
}

Stream* callbackNextStream(int offset) {
  return throttle;
}

void setup() {
  Serial.begin(115200);
  AudioLogger::instance().begin(Serial, AudioLogger::Info);
  // setup file
  //SD.begin(chipSelect);
  //audioFile = SD.open("/Music/Elephunk.mp3");
  Serial2.begin(3000000, SERIAL_8N1 ,RXD2,TXD2);
  auto cfgA2DP = out.defaultConfig(TX_MODE);
  cfgA2DP.name = "SRS-XB13";
  //cfgA2DP.auto_reconnect = false;
  out.begin(cfgA2DP);
  out.setVolume(0.5);
  //copier.begin();
  // setup player
  player.begin();
}

void loop() {
  player.copy();
}
