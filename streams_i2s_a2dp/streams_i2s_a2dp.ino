/**
 * @file streams-i2s-a2dp.ino
 * @author Phil Schatzmann
 * @brief see https://github.com/pschatzmann/arduino-audio-tools/blob/main/examples/examples-stream/streams-i2s-a2dp/README.md
 * 
 * @author Phil Schatzmann
 * @copyright GPLv3
 */

#include "AudioTools.h"
#include "AudioLibs/A2DPStream.h"

I2SStream i2sStream;                            // Access I2S as stream
A2DPStream a2dpStream;                          // access A2DP as stream
StreamCopy copier(a2dpStream, i2sStream); // copy i2sStream to a2dpStream
AudioInfo info(44100, 2, 16);
ConverterFillLeftAndRight<int16_t> filler(LeftIsEmpty); // fill both channels

// Arduino Setup
void setup(void) {
    Serial.begin(115200);
    AudioLogger::instance().begin(Serial, AudioLogger::Info);
    
    // start bluetooth
    Serial.println("starting A2DP...");
    auto cfgA2DP = a2dpStream.defaultConfig(TX_MODE);
    cfgA2DP.name = "SRS-XB13";
    a2dpStream.begin(cfgA2DP);

    // set intial volume
    a2dpStream.setVolume(0.3);

    // start i2s input with default configuration
    Serial.println("starting I2S...");
    a2dpStream.addNotifyAudioChange(i2sStream); // i2s is using the info from a2dp
    auto cfg=i2sStream.defaultConfig(RX_MODE);
    //cfg.copyFrom(info);
    cfg.pin_data_rx=12;
    cfg.pin_bck=14;
    cfg.pin_ws=15;
    cfg.port_no=0;
    cfg.is_master=true;
    cfg.i2s_format=I2S_STD_FORMAT;
    cfg.buffer_size = 256;
    cfg.bits_per_sample =16;
    cfg.buffer_count=8;
    cfg.channels=2;
    cfg.sample_rate=44100;
    i2sStream.begin(cfg);

}

// Arduino loop - copy data
void loop() {
   // copier.copy(filler);
    copier.copy();
}
