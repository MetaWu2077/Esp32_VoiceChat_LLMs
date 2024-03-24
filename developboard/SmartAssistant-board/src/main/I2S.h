#ifndef _I2S_H
#define _I2S_H
#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2s.h"
#include "esp_system.h"

enum MicType {
  ADMP441,
  ICS43434,
  M5GO,
  M5STACKFIRE
};

class I2S {
  i2s_bits_per_sample_t BITS_PER_SAMPLE;
public:
  I2S();
  int Read(char* data, int numData);
  int GetBitPerSample();
  void clear();
};

#endif // _I2S_H
