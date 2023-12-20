#include "common.h"

void switch_uart(module_t module) {
  switch (module) {
    case SR04T:
      digitalWrite(EN_PIN, LOW);
      digitalWrite(A1_PIN, HIGH);
      digitalWrite(A0_PIN, HIGH);
      break;
    case GPS:
      digitalWrite(EN_PIN, LOW);
      digitalWrite(A1_PIN, LOW);
      digitalWrite(A0_PIN, HIGH);
      break;
    case SIM:
      digitalWrite(EN_PIN, LOW);
      digitalWrite(A1_PIN, LOW);
      digitalWrite(A0_PIN, LOW);
      break;
    case DF:
      digitalWrite(EN_PIN, LOW);
      digitalWrite(A1_PIN, HIGH);
      digitalWrite(A0_PIN, LOW);
      break;
    default:
      digitalWrite(EN_PIN, HIGH);
      break;
  }
  delay(20);
}

void waitForSerial(unsigned long timeout_millis) {
  unsigned long start = millis();
  while (!Serial) {
    if (millis() - start > timeout_millis)
      break;
  }
}

void printDebug(String log) {
  switch_uart(USB);
  Serial.println(log);
}