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
//#include "AudioCodecs/CodecADPCM.h"
#include "AudioCodecs/CodecSBC.h"
#include "AudioCodecs/ContainerBinary.h"

#define RXD2 14
#define TXD2 15

auto &serial = Serial2;
A2DPStream out;
AudioInfo info(44100, 2, 16);
SBCDecoder dec;//(AV_CODEC_ID_ADPCM_IMA_WAV);
BinaryContainerDecoder bin_dec(&dec);
//FormatConverterStream conv(i2s);
CsvOutput<int8_t> csvStream(Serial);
EncodedAudioStream dec_stream(&bin_dec);
static int frame_size = 4096;
StreamCopy copierIn(dec_stream, serial, frame_size);

void setup() {
  Serial.begin(115200);
  AudioLogger::instance().begin(Serial, AudioLogger::Info);

  copierIn.setCheckAvailable(true);
  copierIn.setCheckAvailableForWrite(true);

  Serial2.begin(1100000, SERIAL_8N1, RXD2, TXD2);
  dec_stream.setFrameSize(frame_size);
  dec_stream.begin(info);

  /*auto cfgA2DP = out.defaultConfig(TX_MODE);
  cfgA2DP.name = "SRS-XB13";
  //cfgA2DP.auto_reconnect = false;
  out.begin(cfgA2DP);
  out.setVolume(0.5);*/
  
  csvStream.begin(info);
  copierIn.begin();
}

void loop() {
  copierIn.copy();
}
