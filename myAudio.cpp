#include "myAudio.h"
#include "i2s_config.h"
#include <Base64_Arturo.h>
#include <SPIFFS.h>



/**********引用其他文件的变量*************/

uint32_t audio1SampleRate = 0; // 播放音乐时的SampleRate
uint8_t audioVolume = 15;      // 音量
uint32_t audio1FilePos = 0;    // 音频1文件位置
uint32_t audio2FilePos = 0;    // 音频2文件位置

bool audio2_played = false; // 音频2在播放

Audio audio;
Audio audio2;
/// @brief 音频任务结构体
struct audioMessage
{
  uint8_t cmd;
  const char *txt;
  uint32_t value;
  uint32_t ret;
} audioTxMessage, audioRxMessage;

enum : uint8_t
{
  SET_VOLUME,
  GET_VOLUME,
  CONNECTTOHOST,
  CONNECTTOSD,
  CONNECTTOSPIFFS,
  SET_SAMPLE_RETES,
  GET_SAMPLE_RETES,
  GET_FILE_POS,
  PAUSE,
  STOP
};

/// // 音频1任务设置队列
QueueHandle_t audioSetQueue = NULL; // 设置队列
QueueHandle_t audioGetQueue = NULL; // 获取队列

/// // 音频2任务设置队列
QueueHandle_t audio2SetQueue = NULL; // 设置队列
QueueHandle_t audio2GetQueue = NULL; // 获取队列


void audio_info(const char *info)
{
  Serial.print("info        ");
  Serial.println(info);
}

void audio_id3data(const char *info)
{ // id3 metadata
  Serial.print("id3data     ");
  Serial.println(info);
}

void audio_eof_mp3(const char *info)
{ // end of file
  Serial.print("eof_mp3     ");
  Serial.println(info);

}

void audio_showstation(const char *info)
{
  Serial.print("station     ");
  Serial.println(info);
}

void audio_showstreamtitle(const char *info)
{
  Serial.print("streamtitle ");
  Serial.println(info);
}

void audio_bitrate(const char *info)
{
  Serial.print("bitrate     ");
  Serial.println(info);
}

void audio_commercial(const char *info)
{ // duration in sec
  Serial.print("commercial  ");
  Serial.println(info);
}

void audio_icyurl(const char *info)
{ // homepage
  Serial.print("icyurl      ");
  Serial.println(info);
}

void audio_lasthost(const char *info)
{ // stream URL played
  Serial.print("lasthost    ");
  Serial.println(info);
}

void audio_eof_speech(const char *info)
{
  Serial.print("eof_speech  ");
  Serial.println(info);
}

void audio_eof_stream(const char *info)
{
  Serial.print("end of web stream  ");
  Serial.println(info);
}


// Play audio data using MAX98357A
void playAudio(uint8_t *audioData, size_t audioDataSize)
{
  if (audioDataSize > 0)
  {
    // 发送
    size_t bytes_written = 0;
    i2s_write(I2S_PORT_0, (int16_t *)audioData, audioDataSize, &bytes_written, portMAX_DELAY);
  }
}


void clearAudio(void)
{
  // 清空I2S DMA缓冲区
  i2s_zero_dma_buffer(I2S_PORT_0);
  Serial.print("clearAudio");
}

/// @brief 播放base64音频
/// @param  
void playAudio_Zai(void)
{
  const char *zai = ""; // base64音频数据
  // 分配内存
  uint8_t *decode_data = (uint8_t *)ps_malloc(16000 * 3);
  if (!decode_data)
  {
    Serial.println("Failed to allocate memory for decode_data");
    return;
  }

  // base64 解析
  int decoded_length = Base64_Arturo.decode((char *)decode_data, (char *)zai, 28220);

  // 播放
  playAudio(decode_data, decoded_length);

  // delay 200ms
  delay(200);

  // 清空I2S DMA缓冲区
  clearAudio();

  // 释放内存
  free(decode_data);
}


/// @brief 创建队列
void CreateQueues()
{
  audioSetQueue = xQueueCreate(10, sizeof(struct audioMessage));
  audioGetQueue = xQueueCreate(10, sizeof(struct audioMessage));
}


/// @brief 音频任务任务
/// @param arg
void audioTask(void *arg)
{
  CreateQueues();
  if (!audioSetQueue || !audioGetQueue)
  {
    log_e("queues are not initialized");
    while (true)
    {
      ;
    } // endless loop
  }
  struct audioMessage audioRxTaskMessage;
  struct audioMessage audioTxTaskMessage;

  audio.setPinout(MAX98357_BCLK, MAX98357_LRC, MAX98357_DIN);
  audio.setVolume(audioVolume); // 0...21
  //SPIFFS.format();
  bool spiffsOK = SPIFFS.begin(true);
  if(!spiffsOK)
  {
    log_e("SPIFFS not initialized");
  }
  else{
   
  }
  while (true)
  {
    if (xQueueReceive(audioSetQueue, &audioRxTaskMessage, 1) == pdPASS) // 查询队列是否有消息
    {
      if (audioRxTaskMessage.cmd == SET_VOLUME)
      {
        audioTxTaskMessage.cmd = SET_VOLUME;
        audio.setVolume(audioRxTaskMessage.value);
        audioTxTaskMessage.ret = 1;
        xQueueSend(audioGetQueue, &audioTxTaskMessage, portMAX_DELAY);
      }
      else if (audioRxTaskMessage.cmd == GET_VOLUME)
      {
        audioTxTaskMessage.cmd = GET_VOLUME;
        audioTxTaskMessage.ret = audio.getVolume();
        xQueueSend(audioGetQueue, &audioTxTaskMessage, portMAX_DELAY);
      }
      else if (audioRxTaskMessage.cmd == SET_SAMPLE_RETES)
      {
        audioTxTaskMessage.cmd = SET_SAMPLE_RETES;
        audio.setNewSampleRate(audioRxTaskMessage.value);
        audioTxTaskMessage.ret = 1;
        xQueueSend(audioGetQueue, &audioTxTaskMessage, portMAX_DELAY);
      }
      else if (audioRxTaskMessage.cmd == GET_SAMPLE_RETES)
      {
        audioTxTaskMessage.cmd = GET_SAMPLE_RETES;
        audioTxTaskMessage.ret = audio.getSampleRate();
        xQueueSend(audioGetQueue, &audioTxTaskMessage, portMAX_DELAY);
      }
      else if (audioRxTaskMessage.cmd == GET_FILE_POS)
      {
        audioTxTaskMessage.cmd = GET_FILE_POS;
        audioTxTaskMessage.ret = audio.getFilePos() - audio.inBufferFilled(); //计算文件播放位置
        xQueueSend(audioGetQueue, &audioTxTaskMessage, portMAX_DELAY);
      }
      else if (audioRxTaskMessage.cmd == CONNECTTOHOST)
      {
        audioTxTaskMessage.cmd = CONNECTTOHOST;
        audioTxTaskMessage.ret = audio.connecttohost(audioRxTaskMessage.txt);
        xQueueSend(audioGetQueue, &audioTxTaskMessage, portMAX_DELAY);
      }
      else if (audioRxTaskMessage.cmd == CONNECTTOSD)
      {
        audioTxTaskMessage.cmd = CONNECTTOSD;
        audioTxTaskMessage.ret = audio.connecttoSD(audioRxTaskMessage.txt, audioRxTaskMessage.value);
        xQueueSend(audioGetQueue, &audioTxTaskMessage, portMAX_DELAY);
      }
      else if (audioRxTaskMessage.cmd == CONNECTTOSPIFFS)
      {
        audioTxTaskMessage.cmd = CONNECTTOSPIFFS;
        audioTxTaskMessage.ret = audio.connecttoFS2(SPIFFS, audioRxTaskMessage.txt);
        xQueueSend(audioGetQueue, &audioTxTaskMessage, portMAX_DELAY);
      }

      else if (audioRxTaskMessage.cmd == PAUSE)
      {
        audioTxTaskMessage.cmd = PAUSE;
        audioTxTaskMessage.ret = audio.pauseResume();
        xQueueSend(audioGetQueue, &audioTxTaskMessage, portMAX_DELAY);
      }
      else if (audioRxTaskMessage.cmd == STOP)
      {
        audioTxTaskMessage.cmd = STOP;
        audioTxTaskMessage.ret = audio.stopSong();
        xQueueSend(audioGetQueue, &audioTxTaskMessage, portMAX_DELAY);
      }
      else
      {
        log_i("error");
      }

    }
    audio.loop();
    if (!audio.isRunning())
    {
      sleep(1);
    }
  }
  vTaskDelete(NULL);
}

/// @brief 队列1发送接收
/// @param msg 消息
/// @return 
audioMessage transmitReceive(audioMessage msg)
{
  xQueueSend(audioSetQueue, &msg, portMAX_DELAY);
  if (xQueueReceive(audioGetQueue, &audioRxMessage, portMAX_DELAY) == pdPASS)
  {
    if (msg.cmd != audioRxMessage.cmd)
    {
      log_e("wrong reply from message queue");
    }
  }
  return audioRxMessage;
}

/// @brief 队列2发送接收
/// @param msg 消息
/// @return 
audioMessage transmitReceive2(audioMessage msg)
{
  xQueueSend(audio2SetQueue, &msg, portMAX_DELAY);
  if (xQueueReceive(audio2GetQueue, &audioRxMessage, portMAX_DELAY) == pdPASS)
  {
    if (msg.cmd != audioRxMessage.cmd)
    {
      log_e("wrong reply from message queue");
    }
  }
  return audioRxMessage;
}


/// @brief 设置音频采样率
/// @param id 音频任务id 1 2
/// @param rates 采样率 8000 16000 22050 44100
void audioSetSampleRates(uint8_t id, uint32_t rates)
{
  audioTxMessage.cmd = SET_SAMPLE_RETES;
  audioTxMessage.value = rates;
  if(id == 1)
  {
    audioMessage RX = transmitReceive(audioTxMessage);
    if(!RX.ret) Serial.println("SetSampleRates failed!");
  }
  else
  {
    audioMessage RX = transmitReceive2(audioTxMessage);
    if(!RX.ret) Serial.println("SetSampleRates failed!");
  }
}


/// @brief 获取音频采样率
/// @param id 音频任务id 1 2
/// @return 采样率
uint32_t audioGetSampleRates(uint8_t id)
{
  audioTxMessage.cmd = GET_SAMPLE_RETES;
  if(id == 1)
  {
    audioMessage RX = transmitReceive(audioTxMessage);
    return RX.ret;
  }
  else
  {
    audioMessage RX = transmitReceive2(audioTxMessage);
    return RX.ret;
  }
}

/// @brief 获取文件位置
uint32_t audioGetFilePos(uint8_t id)
{
  audioTxMessage.cmd = GET_FILE_POS;
  if(id == 1)
  {
    audioMessage RX = transmitReceive(audioTxMessage);
    return RX.ret;
  }
  else
  {
    audioMessage RX = transmitReceive2(audioTxMessage);
    return RX.ret;
  }
} 

/// @brief 设置音量
/// @param id 音频任务id 1 2
/// @param vol 音量 0 -21
void audioSetVolume(uint8_t id,uint8_t vol)
{
  audioTxMessage.cmd = SET_VOLUME;
  audioTxMessage.value = vol;
  if(id == 1)
  {
    audioMessage RX = transmitReceive(audioTxMessage);
  }
  else
  {
    audioMessage RX = transmitReceive2(audioTxMessage);
  }
}

/// @brief 获取音量
/// @param id 音频任务id 1 2
/// @return 音量数字
uint8_t audioGetVolume(uint8_t id)
{
  audioTxMessage.cmd = GET_VOLUME;
  if(id==1)
  {
    audioMessage RX = transmitReceive(audioTxMessage);
    return RX.ret;
  }
  else
  {
    audioMessage RX = transmitReceive2(audioTxMessage);
    return RX.ret;
  }
}

/// @brief 播放MP3资源
/// @param id 音频任务id 1 2
/// @param host url
/// @return 
bool audioConnecttohost(uint8_t id,const char *host)
{
  audioTxMessage.cmd = CONNECTTOHOST;
  audioTxMessage.txt = host;
  if(id == 1)
  {
    audioMessage RX = transmitReceive(audioTxMessage);
    return RX.ret;
  }
  else
  {
    audioMessage RX = transmitReceive2(audioTxMessage);
    return RX.ret;
  }

}

/// @brief 播放sd卡的音乐
/// @param id 音频任务id 1 2
/// @param filename 文件名称
/// @param pos 文件w位置
/// @return 
bool audioConnecttoSD(uint8_t id,const char *filename, uint32_t pos)
{
  audioTxMessage.cmd = CONNECTTOSD;
  audioTxMessage.txt = filename;
  audioTxMessage.value = pos;
  if(id == 1)
  {
    audioMessage RX = transmitReceive(audioTxMessage);
    return RX.ret;
  }
  else
  {
    audioMessage RX = transmitReceive2(audioTxMessage);
    return RX.ret;
  }

}


/// @brief 播发spiffs的音乐
/// @param id 音频任务id 1 2
/// @param filename 文件名称
/// @return 
bool audioConnecttoSPIFFS(uint8_t id,const char *filename)
{
  audioTxMessage.cmd = CONNECTTOSPIFFS;
  audioTxMessage.txt = filename;
  if(id == 1)
  {
    audioMessage RX = transmitReceive(audioTxMessage);
    return RX.ret;
  }
  else
  {
    audioMessage RX = transmitReceive2(audioTxMessage);
    return RX.ret;
  }

}


/// @brief 停止音频
/// @param id 音频任务id 1, 2
/// @return 
bool audioStopSong(uint8_t id)
{
  audioTxMessage.cmd = STOP;
  if(id == 1)
  {
    audioMessage RX = transmitReceive(audioTxMessage);
    return RX.ret;
  }
  else
  {
    audioMessage RX = transmitReceive2(audioTxMessage);
    return RX.ret;
  }
}

/// @brief 音频暂停/继续
/// @param id 音频任务id 1, 2
/// @return 
bool audioPause(uint8_t id)
{
  audioTxMessage.cmd = PAUSE;
  if(id == 1)
  {
    audioMessage RX = transmitReceive(audioTxMessage);
    return RX.ret;
  }
  else
  {
    audioMessage RX = transmitReceive2(audioTxMessage);
    return RX.ret;
  }

}

/// @brief 是否正在播放
/// @param id 音频任务id 1, 2
/// @return true or false
bool audioIsplaying(uint8_t id)
{
  if(id == 1)
  {
    return audio.isRunning();
  }
  else
  {
    return audio2.isRunning();
  }

}

/// @brief 初始化audio
void audioInit()
{
  xTaskCreatePinnedToCore(
      audioTask,             /* Function to implement the task */
      "audioplay",           /* Name of the task */
      8192,                  /* Stack size in words */
      NULL,                  /* Task input parameter */
      2 | portPRIVILEGE_BIT, /* Priority of the task */
      NULL,                  /* Task handle. */
      1                      /* Core where the task should run */
  );
 
}


/// @brief 创建队列2
void CreateQueues2()
{
  audio2SetQueue = xQueueCreate(10, sizeof(struct audioMessage));
  audio2GetQueue = xQueueCreate(10, sizeof(struct audioMessage));
}

/// @brief 音频2任务任务
/// @param arg
void audio2Task(void *arg)
{
  CreateQueues2();
  if (!audio2SetQueue || !audio2GetQueue)
  {
    log_e("queues are not initialized");
    while (true)
    {
      ;
    } // endless loop
  }
  struct audioMessage audioRxTaskMessage;
  struct audioMessage audioTxTaskMessage;
   
   // I2S初始化在audio1Task中已经初始化
  // audio.setPinout(MAX98357_BCLK, MAX98357_LRC, MAX98357_DIN);
  // audio.setVolume(audioVolume); // 0...21
  
  while (true)
  {
    if (xQueueReceive(audio2SetQueue, &audioRxTaskMessage, 1) == pdPASS) // 查询队列是否有消息
    {
      if (audioRxTaskMessage.cmd == SET_VOLUME)
      {
        audioTxTaskMessage.cmd = SET_VOLUME;
        audio2.setVolume(audioRxTaskMessage.value);
        audioTxTaskMessage.ret = 1;
        xQueueSend(audio2GetQueue, &audioTxTaskMessage, portMAX_DELAY);
      }
      else if (audioRxTaskMessage.cmd == GET_VOLUME)
      {
        audioTxTaskMessage.cmd = GET_VOLUME;
        audioTxTaskMessage.ret = audio2.getVolume();
        xQueueSend(audio2GetQueue, &audioTxTaskMessage, portMAX_DELAY);
      }
      else if (audioRxTaskMessage.cmd == SET_SAMPLE_RETES)
      {
        audioTxTaskMessage.cmd = SET_SAMPLE_RETES;
        audio2.setNewSampleRate(audioRxTaskMessage.value);
        audioTxTaskMessage.ret = 1;
        xQueueSend(audio2GetQueue, &audioTxTaskMessage, portMAX_DELAY);
      }
      else if (audioRxTaskMessage.cmd == GET_SAMPLE_RETES)
      {
        audioTxTaskMessage.cmd = GET_SAMPLE_RETES;
        audioTxTaskMessage.ret = audio2.getSampleRate();
        xQueueSend(audio2GetQueue, &audioTxTaskMessage, portMAX_DELAY);
      }
      else if (audioRxTaskMessage.cmd == GET_FILE_POS)
      {
        audioTxTaskMessage.cmd = GET_FILE_POS;
        audioTxTaskMessage.ret = audio2.getFilePos();
        xQueueSend(audio2GetQueue, &audioTxTaskMessage, portMAX_DELAY);
      }
      else if (audioRxTaskMessage.cmd == CONNECTTOHOST)
      {
        audioTxTaskMessage.cmd = CONNECTTOHOST;
        audioTxTaskMessage.ret = audio2.connecttohost(audioRxTaskMessage.txt);
        xQueueSend(audio2GetQueue, &audioTxTaskMessage, portMAX_DELAY);
      }
      else if (audioRxTaskMessage.cmd == CONNECTTOSD)
      {
        audioTxTaskMessage.cmd = CONNECTTOSD;
        audioTxTaskMessage.ret = audio2.connecttoSD(audioRxTaskMessage.txt, audioRxTaskMessage.value);
        xQueueSend(audio2GetQueue, &audioTxTaskMessage, portMAX_DELAY);
      }
      else if (audioRxTaskMessage.cmd == CONNECTTOSPIFFS)
      {
        audioTxTaskMessage.cmd = CONNECTTOSPIFFS;
        audioTxTaskMessage.ret = audio2.connecttoFS2(SPIFFS, audioRxTaskMessage.txt);
        xQueueSend(audio2GetQueue, &audioTxTaskMessage, portMAX_DELAY);
      }

      else if (audioRxTaskMessage.cmd == PAUSE)
      {
        audioTxTaskMessage.cmd = PAUSE;
        audioTxTaskMessage.ret = audio2.pauseResume() - audio2.inBufferFilled(); //计算文件播放位置;
        xQueueSend(audio2GetQueue, &audioTxTaskMessage, portMAX_DELAY);
      }
      else if (audioRxTaskMessage.cmd == STOP)
      {
        audioTxTaskMessage.cmd = STOP;
        audioTxTaskMessage.ret = audio2.stopSong();
        xQueueSend(audio2GetQueue, &audioTxTaskMessage, portMAX_DELAY);
      }
      else
      {
        log_i("error");
      }

    }
    audio2.loop();
    if (!audio2.isRunning())
    {
      sleep(1);
    }
  }
  vTaskDelete(NULL);
}


/// @brief 初始化audio
void audio2Init()
{
  xTaskCreatePinnedToCore(
      audio2Task,             /* Function to implement the task */
      "audio2play",           /* Name of the task */
      8192,                  /* Stack size in words */
      NULL,                  /* Task input parameter */
      2 | portPRIVILEGE_BIT, /* Priority of the task */
      NULL,                  /* Task handle. */
      1                      /* Core where the task should run */
  );
 
}