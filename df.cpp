
#include "df.h"

DFRobotDFPlayerMini myDFPlayer;

void df_init() {
  printDebug("df_init");
  switch_uart(DF);
  myDFPlayer.begin(FPSerial, false, false);
  myDFPlayer.setTimeOut(500);
  myDFPlayer.volume(4);
  myDFPlayer.EQ(DFPLAYER_EQ_NORMAL);
  myDFPlayer.outputDevice(DFPLAYER_DEVICE_SD);
}

void alarm() {
  printDebug("alarm");
  switch_uart(DF);
  myDFPlayer.playMp3Folder(1); //play specific mp3 in SD:/MP3/0001.mp3; File Name(0~65535)
  delay(1000);
}

void askUser() {
  printDebug("askUser");
  switch_uart(DF);
  myDFPlayer.playMp3Folder(2); //play specific mp3 in SD:/MP3/0002.mp3; File Name(0~65535)
  delay(2000);
}

void stopDF() {
  switch_uart(DF);
  myDFPlayer.stop();
}

void add_sdt_success() {
  printDebug("add_sdt_success");
  switch_uart(DF);
  myDFPlayer.playMp3Folder(3); //play specific mp3 in SD:/MP3/0002.mp3; File Name(0~65535)
  delay(2000);
}

void reset_sdt_success() {
  printDebug("reset_sdt_success");
  switch_uart(DF);
  myDFPlayer.playMp3Folder(4); //play specific mp3 in SD:/MP3/0002.mp3; File Name(0~65535)
  delay(2000);
}
