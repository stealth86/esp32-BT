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
#include "AudioCodecs/CodecMP3Helix.h"
#include "AudioLibs/AudioSourceSDMMC.h"

const char* startFilePath = "/Music/";
const char* ext = "mp3";
AudioSourceSDMMC source(startFilePath, ext);
A2DPStream out;
MP3DecoderHelix decoder;
AudioPlayer player(source, out, decoder);

void setup() {
  Serial.begin(115200);
  AudioLogger::instance().begin(Serial, AudioLogger::Info);
  // setup file
  //SD.begin(chipSelect);
  //audioFile = SD.open("/Music/Elephunk.mp3");
  auto cfgA2DP = out.defaultConfig(TX_MODE);
  cfgA2DP.name = "SRS-XB13";
  //cfgA2DP.auto_reconnect = false;
  out.begin(cfgA2DP);
  out.setVolume(0.5);

  // setup player
  player.begin();
}

void loop() {
  player.copy();
}
