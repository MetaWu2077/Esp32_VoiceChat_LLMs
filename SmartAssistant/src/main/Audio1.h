#ifndef _AUDIO_H
#define _AUDIO_H

#include <Arduino.h>
#include "base64.h"
#include "I2S.h"
#include <WiFi.h>
#include <ArduinoJson.h>

// 16bit, monoral, 16000Hz,  linear PCM
class Audio1
{
  I2S *i2s;
  static const int headerSize = 44;
  static const int i2sBufferSize = 5120;
  char i2sBuffer[i2sBufferSize];
  const int DATA_JSON_SIZE = 512;
  char data_json[512];
  String Question = "";

  WiFiClient client; // @suppress("Abstract class cannot be instantiated")
  // WiFiClientSecure clientsecure; // @suppress("Abstract class cannot be instantiated")
  WiFiClient *_client = nullptr;
  const char *header_format = "{"
                              "\"format\":\"pcm\","
                              "\"rate\":8000,"
                              "\"dev_pid\":1537,"
                              "\"channel\":1,"
                              "\"cuid\":\"\","
                              "\"token\":\"\","
                              "\"speech\":\"";

  const char *footer_format = "\",\"len\":";
  void CreateWavHeader(byte *header, int waveDataSize);
  String parseJSON(const char *jsonResponse);
  float calculateRMS(uint8_t *buffer, int bufferSize);

public:
  static const int wavDataSize = 30000; // It must be multiple of dividedWavDataSize. Recording time is about 1.9 second.
  static const int dividedWavDataSize = i2sBufferSize / 4;
  char **wavData;                          // It's divided. Because large continuous memory area can't be allocated in esp32.
  byte paddedHeader[headerSize + 4] = {0}; // The size must be multiple of 3 for Base64 encoding. Additional byte size must be even because wave data is 16bit.

  Audio1();
  ~Audio1();
  void Record();
  void clear();
  void init();
};

#endif // _AUDIO_H
