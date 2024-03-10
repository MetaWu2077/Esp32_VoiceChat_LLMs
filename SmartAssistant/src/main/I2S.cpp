#include "I2S.h"
#define SAMPLE_RATE (8000)
#define PIN_I2S_BCLK 14
#define PIN_I2S_LRC 15
#define PIN_I2S_DIN 32
// #define PIN_I2S_DOUT 25

const i2s_port_t I2S_PORT = I2S_NUM_0;
const int BLOCK_SIZE = 128;
// This I2S specification :
//  -   LRC high is channel 2 (right).
//  -   LRC signal transitions once each word.
//  -   DATA is valid on the CLOCK rising edge.
//  -   Data bits are MSB first.
//  -   DATA bits are left-aligned with respect to LRC edge.
//  -   DATA bits are right-shifted by one with respect to LRC edges.
I2S::I2S()
{

  BITS_PER_SAMPLE = I2S_BITS_PER_SAMPLE_32BIT;
  i2s_config_t i2s_config = {
      .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
      .sample_rate = SAMPLE_RATE,
      .bits_per_sample = BITS_PER_SAMPLE,
      .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
      .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
      .intr_alloc_flags = 0,
      .dma_buf_count = 16,
      .dma_buf_len = 60};
  i2s_pin_config_t pin_config;
  pin_config.bck_io_num = PIN_I2S_BCLK;
  pin_config.ws_io_num = PIN_I2S_LRC;
  pin_config.data_out_num = I2S_PIN_NO_CHANGE;
  pin_config.data_in_num = PIN_I2S_DIN;
  pin_config.mck_io_num = GPIO_NUM_0; // Set MCLK to GPIO0
  i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM_0, &pin_config);
  i2s_set_clk(I2S_NUM_0, SAMPLE_RATE, BITS_PER_SAMPLE, I2S_CHANNEL_STEREO);
}

int I2S::Read(char *data, int numData)
{
  size_t bytesRead;
  i2s_read(I2S_NUM_0, (char *)data, numData, &bytesRead, portMAX_DELAY);
  return bytesRead;
}

int I2S::GetBitPerSample()
{
  return (int)BITS_PER_SAMPLE;
}

void I2S::clear()
{
  i2s_zero_dma_buffer(I2S_NUM_0);
}
