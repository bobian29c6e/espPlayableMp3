#include <Arduino.h>
#include "myAudio.h"
#include "SPI.h"


// --- 配置区 ---
// #define SD_CS 5     // SD卡CS（片选）引脚，
#define MAX_FILES 50 // 最多扫描50个MP3文件
#define INIT_VOLUME 12 // 初始音量（范围0~21）


String mp3List[MAX_FILES];     // 保存扫描到的MP3文件路径
int mp3Count = 0;              // MP3文件总数
int currentTrack = 0;          // 当前播放曲目的索引
int volume = INIT_VOLUME;      // 当前音量
bool isPaused = false;         // 是否处于暂停状态


/***************************************************
 * 函数：playTrack
 * 功能：播放指定索引的MP3文件
 ***************************************************/
void playTrack(int index) {
  if (index < 0 || index >= mp3Count) return; // 防止越界

  Serial.print(" 播放：");
  Serial.println(mp3List[index]);

  audioStopSong(1); // 停止当前播放
  audioSetVolume(1, volume); // 设置音量
  audioConnecttoSD(1, mp3List[index].c_str()); // 开始播放新文件
  isPaused = false; // 标记为播放状态
}


/***************************************************
 * 函数：handleSerialCommand
 * 功能：处理串口发送过来的控制命令
 ***************************************************/
void handleSerialCommand() {
  if (Serial1.available()) {
    String recvName = Serial1.readStringUntil('\n'); // 读取一行串口内容，以回车换行结束
    recvName.trim(); // 去除首尾空格和换行符
    if (recvName.length() == 0) {
      return;
    } else if (recvName == "999") {
      if (volume < 19) volume = volume + 3;
      audioSetVolume(1,volume);
      Serial.print("音量增加，当前音量：");
      Serial.println(volume);
      return;
    } else if (recvName == "998") {
      if (volume > 3) volume = volume - 3;
      audioSetVolume(1,volume);
      Serial.print(" 音量减少，当前音量：");
      Serial.println(volume);
      return;
    }

    String fileName = "[" + recvName + "]" + ".mp3";
    Serial.print("准备播放文件：");
    Serial.println(fileName);

    audioStopSong(1);
    audioSetVolume(1, volume);
    audioConnecttoSD(1, fileName.c_str());
    isPaused = false;
  }
}

/***************************************************
 * 函数：scanRootMP3s
 * 功能：只扫描根目录下的MP3文件，提高扫描速度
 ***************************************************/
void scanSDForMP3s(File dir, String path = "") {
  // 保留原函数的参数形式，但只扫描根目录
  mp3Count = 0; // 重置计数器
  
  // 强制打开根目录，忽略传入的dir参数
  File root = SD.open("/");
  if (!root) {
    Serial.println("打开根目录失败");
    return;
  }
  
  unsigned long startTime = millis();
  int fileCount = 0;
  
  // 非递归，只扫描根目录
  while (true) {
    File entry = root.openNextFile();
    if (!entry) {
      break; // 没有更多文件，结束
    }
    
    fileCount++;
    
    // 只处理文件，跳过目录
    if (!entry.isDirectory()) {
      String filename = entry.name();
      filename.toLowerCase();
      
      // 跳过Mac生成的._开头的隐藏文件
      if (filename.startsWith("._")) {
        entry.close();
        continue;
      }
      
      // 检查是否为MP3文件
      if (filename.endsWith(".mp3")) {
        if (mp3Count < MAX_FILES) {
          mp3List[mp3Count++] = "/" + filename;
        }
      }
    }
    entry.close();
  }
  root.close();
  
  unsigned long scanTime = millis() - startTime;
  Serial.printf("扫描完成：找到%d个MP3文件，总共%d个文件，用时%lums\n", 
              mp3Count, fileCount, scanTime);
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial1.setPins(44, 43, -1, -1);//修改指定引脚   RXD - ASP-PRO 的PB5,   TXD - ASP-PRO的PB6
  Serial1.begin(9600); // 初始化软件串口
  delay(5000);
  if (!SD.begin()) {
    Serial.println("SD卡初始化失败!");
    return;
  }
  Serial.println(" hello  serial");
  Serial.println("SD卡初始化成功");
  
  audioInit();
  audio2Init();

    // 扫描SD卡里面的MP3文件
    File root = SD.open("/");
    scanSDForMP3s(root);
    root.close();
  
    // 打印扫描到的文件列表
    Serial.println("找到的MP3文件:");
    for (int i = 0; i < mp3Count; i++) {
      Serial.println(mp3List[i]);
    }
}

void loop() {
  handleSerialCommand();    // 处理串口控制指令
}

