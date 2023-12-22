#ifndef COMMON_H
#define COMMON_H

#include <Arduino.h>

#define DEBUG
#define MOTOR_PIN 11
#define EN_PIN 4
#define A1_PIN 3
#define A0_PIN 2
#define BUTTON_PIN 10

#define isButtonPress (digitalRead(BUTTON_PIN) == LOW)
#define MAX_SOS_NUM 3

typedef enum {
  MPU6050,
  SR04T,
  GPS,
  SIM,
  DF,
  USB
} module_t;

const int PHONEBOOK_SIZE = 10; 

void switch_uart(module_t module);
void waitForSerial(unsigned long timeout_millis);
void printDebug(String);

#endif