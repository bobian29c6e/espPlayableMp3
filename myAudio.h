#ifndef MYAUDIO_H
#define MYAUDIO_H
#include <Arduino.h>
// 此工程对Audio库进行了更改，移植请注意！！！
#include "Audio.h"


void audioSetVolume(uint8_t id, uint8_t vol);

uint8_t audioGetVolume(uint8_t id);
bool audioConnecttohost(uint8_t id, const char *host);
bool audioConnecttoSD(uint8_t id,const char *filename, uint32_t pos=0);
bool audioConnecttoSPIFFS(uint8_t id, const char *filename);
bool audioStopSong(uint8_t id);
bool audioPause(uint8_t id);
void audioInit();
uint32_t audioGetSampleRates(uint8_t id);
uint32_t audioGetFilePos(uint8_t id);
void audioSetSampleRates(uint8_t id, uint32_t rates);
bool audioIsplaying(uint8_t id);
void audio2Init();
extern Audio audio;
extern Audio audio2;
#endif