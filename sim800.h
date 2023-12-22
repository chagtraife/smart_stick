#ifndef SIM800_H
#define SIM800_H

#include "common.h"
#include "df.h"
#define SIM800L Serial

void updateSubcriber();
bool addToPhonebook(String name, String phoneNumber, int index);
bool deletePhonebook(int index);
String getContactInfo(int index);
bool deleteSMS(int index);
String readSMS(int index);
void sendSMS(String number, String message);


String sendCMD(String cmd);
String getResponse();
void waitSIM800L(uint16_t timeout);

#endif