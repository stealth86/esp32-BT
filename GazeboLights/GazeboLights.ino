/// @file    Fire2012WithPalette.ino
/// @brief   Simple one-dimensional fire animation with a programmable color palette
/// @example Fire2012WithPalette.ino

#include <FastLED.h>
#include "AudioTools.h"
#include "AudioLibs/AudioRealFFT.h"
#include "AudioLibs/A2DPStream.h"
#include "BluetoothA2DPSink.h"

#define LED_PIN 5
#define COLOR_ORDER RGB
#define CHIPSET WS2812B
#define NUM_LEDS_EAST 84
#define NUM_LEDS_SOUTH 104
#define NUM_LEDS_WEST 84
#define NUM_LEDS_NORTH 104
#define NUM_LEDS 376
#define FRAMES_PER_SECOND 60
#define MAX_MAGNITUDE 44100
#define TOTAL_GRADS 3
#define RXD2 16
#define TXD2 17

A2DPStream a2dp_stream;
AudioRealFFT fft;
CRGB source1[NUM_LEDS];
CRGB source2[NUM_LEDS];
CRGB leds[NUM_LEDS];
float magnitude;
bool useSource1 = false;
uint8_t blendAmount = 0;
uint8_t patternCounter = 0;
uint8_t source1Pattern = 0;
uint8_t source2Pattern = 1;
std::pair<float, float> input(0, MAX_MAGNITUDE);
std::pair<float, float> palette_lookup(0, 255);

auto &serial = Serial2;

AudioInfo info;
Throttle throttle(serial);

//APTXEncoder flac_encoder;

//EncodedAudioStream encoder(&flac_encoder);
StreamCopy copier(throttle,a2dp_stream);

DEFINE_GRADIENT_PALETTE(blue_to_green_to_white_p){
  0, 255, 0, 0,   /* at index 0,   black(0,0,0) */
  128, 0, 255, 0, /* at index 192, red(255,0,0) */
  255, 0, 0, 255  /* at index 255, white(255,255,255) */
};

CRGBPalette16 gPal[3] = { blue_to_green_to_white_p, HeatColors_p, OceanColors_p };

void setup() {

  Serial.begin(115200);
  AudioLogger::instance().begin(Serial, AudioLogger::Info);
  // Setup FFT
  Serial2.begin(115200, SERIAL_8N1,RXD2,TXD2);

  auto cfg=a2dp_stream.defaultConfig(RX_MODE);
  cfg.name="Gazebo LED";

  auto tcfg = fft.defaultConfig();
  tcfg.length = 1024;
  tcfg.channels = 2;
  tcfg.sample_rate = a2dp_stream.sink().sample_rate();
  tcfg.bits_per_sample = 16;
  tcfg.callback = &fftResult;
  fft.begin(tcfg);

  /*info.bits_per_sample=tcfg.bits_per_sample;
  info.sample_rate=tcfg.sample_rate;
  info.channels=tcfg.channels;
  auto enc_cfg = encoder.defaultConfig();
  enc_cfg.copyFrom(info);

  Serial.print("starting Gazebo LED...");
  a2dp_sink.set_auto_reconnect(false);
  a2dp_sink.start("Gazebo LED");
  encoder.begin();*/
  Serial.print("starting Gazebo LED...");
  a2dp_stream.begin(cfg);

  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  a2dp_stream.sink().set_stream_reader(writeDataStream, false);
  copier.begin();
  copier0.begin();
  // Start Bluetooth Audio Receiver
}

void loop() {

  copier.copy();
  copier0.copy();
  EVERY_N_SECONDS(5) {
    nextPattern();
  }

  runPattern(source1Pattern, source1);
  runPattern(source2Pattern, source2);

  EVERY_N_MILLISECONDS(10) {
    blend(source1, source2, leds, NUM_LEDS, blendAmount);

    if (useSource1) {
      if (blendAmount < 255) blendAmount++;
    } else {
      if (blendAmount > 0) blendAmount--;
    }
  }

  FastLED.show();
}

void nextPattern() {
  patternCounter = (patternCounter + 1) % 3;

  if (useSource1) source1Pattern = patternCounter;
  else source2Pattern = patternCounter;

  useSource1 = !useSource1;
}

void runPattern(uint8_t pattern, CRGB *LEDArray) {
  switch (pattern) {
    case 0:
      updateLed(gPal[0], LEDArray);
      break;
    case 1:
      updateLed(gPal[1], LEDArray);
      break;
    case 2:
      updateLed(gPal[2], LEDArray);
      break;
  }
}

template<typename tVal>
tVal map_value(std::pair<tVal, tVal> a, std::pair<tVal, tVal> b, tVal inVal) {
  tVal inValNorm = inVal - a.first;
  tVal aUpperNorm = a.second - a.first;
  tVal normPosition = inValNorm / aUpperNorm;

  tVal bUpperNorm = b.second - b.first;
  tVal bValNorm = normPosition * bUpperNorm;
  tVal outVal = b.first + bValNorm;
  return outVal;
}

void writeDataStream(const uint8_t *data, uint32_t length) {
  fft.write(data, length);
  //rstream.write(data, length);
  //encoder.write(data, length);
  //copier.copy();
  //conv.readBytes(data, length);
}

void fftResult(AudioFFTBase &fft) {
  auto result = fft.result();
  magnitude = result.magnitude;
}

void updateLed(CRGBPalette16 inPal, CRGB *leds) {
  if (magnitude > 100 && magnitude < MAX_MAGNITUDE) {
    int index;
    std::pair<float, float> led_ouput(0, NUM_LEDS_WEST / 2);
    int max_lit = static_cast<int>(round(map_value(input, led_ouput, magnitude)));
    for (int i = 0; i < NUM_LEDS_WEST / 2; i++) {
      leds[i] = leds[i + NUM_LEDS_WEST + NUM_LEDS_SOUTH] = leds[i + NUM_LEDS_WEST + NUM_LEDS_SOUTH + NUM_LEDS_EAST / 2] = leds[i + NUM_LEDS_WEST / 2] = CRGB::Black;
      if (i < max_lit) {
        index = static_cast<int>(round(map_value(std::pair<float, float>(0, max_lit), palette_lookup, static_cast<float>(i))));
        leds[NUM_LEDS_WEST - i - 1] = leds[i] = leds[i + NUM_LEDS_WEST + NUM_LEDS_SOUTH] = leds[NUM_LEDS_WEST + NUM_LEDS_SOUTH + NUM_LEDS_EAST - i - 1] = ColorFromPalette(inPal, index, 100, LINEARBLEND);
      }
    }
    std::pair<float, float> led_ouput1(0, NUM_LEDS_SOUTH / 2);
    max_lit = static_cast<int>(round(map_value(input, led_ouput1, magnitude)));
    for (int i = 0; i < NUM_LEDS_SOUTH / 2; i++) {
      leds[i + NUM_LEDS_WEST] = leds[i + NUM_LEDS_WEST + NUM_LEDS_SOUTH + NUM_LEDS_EAST] = leds[i + NUM_LEDS_WEST + NUM_LEDS_SOUTH + NUM_LEDS_EAST + NUM_LEDS_NORTH / 2] = leds[i + NUM_LEDS_WEST + NUM_LEDS_SOUTH / 2] = CRGB::Black;
      if (i < max_lit) {
        index = static_cast<int>(round(map_value(std::pair<float, float>(0, max_lit), palette_lookup, static_cast<float>(i))));
        //Serial.println(index);
        leds[NUM_LEDS_WEST + NUM_LEDS_SOUTH - i - 1] = leds[i + NUM_LEDS_WEST] = leds[i + NUM_LEDS_WEST + NUM_LEDS_SOUTH + NUM_LEDS_EAST] = leds[NUM_LEDS_WEST + NUM_LEDS_SOUTH + NUM_LEDS_EAST + NUM_LEDS_NORTH - i - 1] = ColorFromPalette(inPal, index, 100, LINEARBLEND);
      }
    }
  }
}