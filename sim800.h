#ifndef SIM800_H
#define SIM800_H

#include "common.h"

void updateSubcriber();
bool addToPhonebook(String name, String phoneNumber, int index);
bool deletePhonebook(int index);
int findEmptyPhonebookIndex();
int findPhoneNumber(String phoneNumber);
String getContactInfo(int index);
bool checkPhonebook(int index);
bool deleteSMS(int index);
String readSMS(int index);
void sendSMS(String number, String message);

#endif