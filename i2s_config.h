#ifndef I2S_CONFIG_H
#define I2S_CONFIG_H
#include <Arduino.h>
#include <driver/i2s.h>

//  INMP441 接口分配
#define I2S_WS 6
#define I2S_SD 4
#define I2S_SCK 5
// Uso del procesador 0 I2S
#define I2S_PORT_1 I2S_NUM_1 // INMP441 使用I2S-1

// MAX98357 接口分配()
#define MAX98357_LRC 2
#define MAX98357_BCLK 42
#define MAX98357_DIN 41

// MAX98357 接口分配(开发板)
// #define MAX98357_LRC 42
// #define MAX98357_BCLK 41
// #define MAX98357_DIN 40

//#define MAX98357_SD 39

#define BUFFER_SIZE  128     // I2S 缓冲区大小
#define CHUNK_SIZE 2048
#define I2S_PORT_0 I2S_NUM_0 // MAX98357 使用I2S-0


/// @brief 配置I2S 引脚
void configureI2S();

void i2sOutConfig();
#endif