#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <TinyGPS.h>
#include "common.h"
#include "sim800.h"
#include "df.h"

Adafruit_MPU6050 mpu;
TinyGPS gps;

void setup() {
  pinMode(EN_PIN, OUTPUT);
  pinMode(A1_PIN, OUTPUT); 
  pinMode(A0_PIN, OUTPUT); 
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  switch_uart(USB);
  Serial.begin(9600);

  // set up i2c
  // Try to initialize!
  if (!mpu.begin()) {
      Serial.println("Failed to find MPU6050 chip");
      while (1) {
        delay(10);
      }
  }
  Serial.println("MPU6050 Found!");
  // set accelerometer range to +-8G
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  // set gyro range to +- 500 deg/s
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  // set filter bandwidth to 21 Hz
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  df_init();
  delay(100);
  if (isButtonPress) {
    if (isLongPress()) {
      bip();
      guide();
      for (int i = 0; i < 30; i++) {
        updateSubcriber();
        delay(1000);
      }
      bip();
    }
  }
}
void read_mpu6050() {
  /* Get new sensor events with the readings */
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  /* Print out the values */
  Serial.print("Acceleration X: ");
  Serial.print(a.acceleration.x);
  Serial.print(", Y: ");
  Serial.print(a.acceleration.y); // dung gia tri nay de kiểm tra xem có ngã hay không, gần = 0 thì ngã
  Serial.print(", Z: ");
  Serial.print(a.acceleration.z);
  Serial.println(" m/s^2");
  delay(500);
}
void read_SR04T() {
  // Array to store incoming serial data
  unsigned char data_buffer[4] = {0};
  // Variable to hold checksum
  unsigned char CS;
  int distance = 0;
  switch_uart(SR04T);
  delay(50);
  Serial.write(0x55);
  Serial.write(0x55);
  waitSIM800L(200);
    // send data only when you receive data:
  if (Serial.available()) {
    // Check for packet header character 0xff
    if (Serial.read() == 0xff) {
      // Insert header into array
      data_buffer[0] = 0xff;
      waitSIM800L(50);
      // Read remaining 3 characters of data and insert into array
      for (int i = 1; i < 4; i++) {
        data_buffer[i] = Serial.read();
        waitSIM800L(50);
      }
 
      //Compute checksum
      CS = data_buffer[0] + data_buffer[1] + data_buffer[2];
      // If checksum is valid compose distance from data
      if (data_buffer[3] == CS) {
        distance = (data_buffer[1] << 8) + data_buffer[2];
        // printDebug("distance: " + String(distance));
        if (distance < 1750) {
          if (distance < 250) distance = 0;
          else distance -= 250;
          int cycle = 255 - uint32_t(distance * 17) / 100;  // 255 - (distance * 255) / 1500;
          // printDebug("cycle: " + String(cycle));
          analogWrite(MOTOR_PIN, cycle);
        } else {
          analogWrite(MOTOR_PIN, 0);
        }
      } else {
        printDebug("======");
      }
    } else {
      printDebug("kkk");
    }
  } else {
        printDebug("+++");
  }
}
bool read_GPS(float *latitude, float *longitude) {
  printDebug("read_GPS");
  unsigned long fix_age;
  switch_uart(GPS);
  bool newData = false;
  // For one second we parse GPS data and report some key values
  for (unsigned long start = millis(); millis() - start < 2000;)
  {
    while (Serial.available())
    {
      char c = Serial.read();
      // Serial.write(c); // uncomment this line if you want to see the GPS data flowing
      if (gps.encode(c)) // Did a new valid sentence come in?
        newData = true;
    }
  }
  if (newData) {
    gps.f_get_position(latitude, longitude);
    // printDebug("latitude");
    // Serial.println(String(*latitude));
    // printDebug("longitude");
    // Serial.println(String(*longitude));
    return true;
  }
  return false;
}
bool checkResponse() {
  int cnt = 0;
  while(!isButtonPress) {
    cnt++;
    delay(50);
    if (cnt > 200) {
      //timeout
      return false;
    }
  }
  return true;
}
bool checkFallResponse() {
  int cnt = 0;
  while((!isButtonPress) && ishorizontal() ) {
    cnt++;
    delay(50);
    if (cnt > 200) {
      //timeout
      return false;
    }
  }
  return true;
}
but_state_t getButState() {
  if (isButtonPress) {
    delay(50);
    int cnt = 0;
    while (isButtonPress && (cnt <50)) {
      cnt++;
      delay(60);
    }
    if (cnt == 50) {
      return LONG_PRESS;
    }
    if ((3 < cnt) && (cnt < 15)) {
      return SHORT_PRESS;
    }
  }
  return NO_STATE;

}
bool isLongPress() {
  if (isButtonPress) {
    delay(50);
    int cnt = 0;
    bool longPress = false;
    while (isButtonPress && (cnt <50)) {
      cnt++;
      if (cnt == 50) {
        longPress = true;
      }
      delay(60);
    }
    if (longPress) {
      return true;
    }
  }
  return false;
}
bool ishorizontal() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  // Serial.print(a.acceleration.y); // dung gia tri nay de kiểm tra xem có ngã hay không, gần = 0 thì ngã
  return (abs(a.acceleration.y) < 2);
}
bool isFall() {
  int cnt = 0;
  while(ishorizontal()) {
    cnt++;
    if (cnt == 50) {
      return true;
    }
    delay(50);
  }
  return false;
}
void sos() {
  printDebug("sos");
  analogWrite(MOTOR_PIN, 0);
  alarm();
  float lat, lon = 0;
  String message = "";
  if (read_GPS(&lat, &lon)) {
    message = "SOS!!! google.com/maps?q=" + String(lat,7) + "," + String(lon,7);
  } else {
    message = "SOS!!!";
  }

  String number = getContactInfo(1);
  if (number.indexOf("+84") != -1) {
    sendSMS(number, message);
  }
}

void loop() {
  read_SR04T();

  if (isButtonPress) {
    if (isLongPress()) {
      printDebug("isLongPress");
      sos();
      while (isButtonPress);
      checkResponse();
      stopDF();
    }
  }

  if (isFall()) {
    askUser();
    if (!checkFallResponse()) {
      sos();
      while (!checkFallResponse());
      stopDF();
    };
  }
  delay(200);
}
