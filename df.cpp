
#include "df.h"

DFRobotDFPlayerMini myDFPlayer;

void df_init() {
  if (!myDFPlayer.begin(FPSerial, /*isACK = */true, /*doReset = */true)) {  //Use serial to communicate with mp3.
    while(true);
  }
  myDFPlayer.setTimeOut(500);
  myDFPlayer.volume(10);
  myDFPlayer.EQ(DFPLAYER_EQ_NORMAL);
  myDFPlayer.outputDevice(DFPLAYER_DEVICE_SD);

}

void alarm() {
  switch_uart(DF);
  myDFPlayer.playMp3Folder(1); //play specific mp3 in SD:/MP3/0001.mp3; File Name(0~65535)
  delay(1000);
}

void askUser() {
  switch_uart(DF);
  myDFPlayer.playMp3Folder(2); //play specific mp3 in SD:/MP3/0002.mp3; File Name(0~65535)
  delay(1000);
}