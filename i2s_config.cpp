#include "i2s_config.h"


/// @brief 配置I2S
void configureI2S() {
  // 配置I2S 输入的音频
  i2s_driver_uninstall(I2S_PORT_1);
    i2s_config_t i2s_config = {
        .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX), // 设置I2S为主模式，接收数据。
        .sample_rate = 16000,   // 设置采样率为16000Hz。
        .bits_per_sample = i2s_bits_per_sample_t(16), // 设置每个样本16位。
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT, // 设置为仅使用左声道。
        .communication_format =  i2s_comm_format_t(I2S_COMM_FORMAT_STAND_I2S),  // 设置通信格式为标准I2S格式。
        .intr_alloc_flags = 0, // 默认中断优先级。
        .dma_buf_count = 8, // 设置DMA缓冲区数量为8。
        .dma_buf_len = 1024, // 设置每个DMA缓冲区长度为bufferLen。
        .use_apll = false,
        .fixed_mclk = 0
    };
      
    esp_err_t err = i2s_driver_install(I2S_PORT_1, &i2s_config, 0, NULL);
    if (err != ESP_OK) {
    Serial.printf("Failed to install I2S driver (I2S_PORT_1): %d\n", err);
    return;
  }

  i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_SCK, 
    .ws_io_num = I2S_WS,  
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = I2S_SD  
  };

   esp_err_t pin_error = i2s_set_pin(I2S_PORT_1, &pin_config);
  if (pin_error != ESP_OK) {
    Serial.printf("Failed to set I2S(I2S_PORT_1) pins: %d\n", pin_error);
    return;
  }
  i2s_start(I2S_PORT_1);
}


void i2sOutConfig(){
  #ifdef MAX98357_SD
    digitalWrite(MAX98357_SD, 1);
  #endif
    i2s_driver_uninstall(I2S_PORT_0);
  // 输出 I2S 配置
    i2s_config_t i2sOut_config = {
    .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_TX),
    .sample_rate = 16000,
    .bits_per_sample = i2s_bits_per_sample_t(16),
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_STAND_I2S),
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 16,
    .dma_buf_len = 1024,
    .tx_desc_auto_clear = true
  };

  esp_err_t err = i2s_driver_install(I2S_PORT_0, &i2sOut_config, 0, NULL);
  if (err != ESP_OK) {
    Serial.printf("I2S driver install failed (I2S_PORT_0): %d\n", err);
    return;
  }

  const i2s_pin_config_t i2sOut_pin_config = {
    .bck_io_num = MAX98357_BCLK,
    .ws_io_num = MAX98357_LRC,
    .data_out_num = MAX98357_DIN,
    .data_in_num = -1
  };

  err = i2s_set_pin(I2S_PORT_0, &i2sOut_pin_config);
  if (err != ESP_OK) {
    Serial.printf("I2S set pin failed (I2S_PORT_0): %d\n", err);
  }
  i2s_start(I2S_PORT_0);
}
