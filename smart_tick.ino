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
  Serial.write(0x55);
  delay(5);
    // send data only when you receive data:
  if (Serial.available() > 0) {
    // Check for packet header character 0xff
    if (Serial.read() == 0xff) {
      // Insert header into array
      data_buffer[0] = 0xff;
      // Read remaining 3 characters of data and insert into array
      for (int i = 1; i < 4; i++) {
        data_buffer[i] = Serial.read();
      }
 
      //Compute checksum
      CS = data_buffer[0] + data_buffer[1] + data_buffer[2];
      // If checksum is valid compose distance from data
      if (data_buffer[3] == CS) {
        distance = (data_buffer[1] << 8) + data_buffer[2];
        if (distance < 1750) {
          if (distance < 250) distance = 0;
          else distance -= 250;
          int cycle = 255 - uint32_t(distance * 17) / 100;  // 255 - (distance * 255) / 1500;
          switch_uart(USB);
          // Serial.print("distance: ");
          // Serial.println(distance);
          Serial.print("cycle: ");
          Serial.println(cycle);
          analogWrite(MOTOR_PIN, cycle);
        } else {
          analogWrite(MOTOR_PIN, 0);
        }
      }
    }
  }
}
bool read_GPS(long *latitude, long *longitude) {
  unsigned long fix_age;
  switch_uart(GPS);
  waitForSerial(2000);
  if (Serial.available()) {
    int c = Serial.read();                   // Read the GPS data
    if (gps.encode(c))                        // Check the GPS data
    {
      // process new gps info here
    }
    gps.get_position(latitude, longitude);
    return true;
  }
  return false;
}
bool checkResponse() {
  int cnt = 0;
  while((!isButtonPress)) {
    cnt++;
    delay(50);
    if (cnt > 100) {
      //timeout
      return false;
    }
  }
  return true;
}
bool isSOSPress() {
  if (isButtonPress) {
    delay(50);
    int cnt = 0;
    bool isLongPress = false;
    while (isButtonPress && (cnt <25)) {
      cnt++;
      if (cnt == 25) {
        isLongPress = true;
      }
      delay(50);
    }
    if (isLongPress) {
      return true;
    }
  }
  return false;
}
bool ishorizontal() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  Serial.print(a.acceleration.y); // dung gia tri nay de kiểm tra xem có ngã hay không, gần = 0 thì ngã
  return (abs(a.acceleration.y) < 1);
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
  long lat, lon;
  if (read_GPS(&lat, &lon)) {
    int cnt = 0;
    for (int i = 0; (i < PHONEBOOK_SIZE) && (cnt < MAX_SOS_NUM); i++) {
      String number = getContactInfo(i);
      if (number != "") {
        cnt++;
        String message = "SOS!!! Please Help me!!! You can find me at location: https://www.google.com/maps?q=" + String(lat) + "," + String(lon);
        sendSMS(number, message);
      }
    }
  }
  alarm();
}

int cnt_loop = 0;
bool isFalled = false;
void loop() {
  read_SR04T();

  if (!isFall()) isFalled = false;
  else if (!isFalled) {
    askUser();
    if (!checkResponse()) {
      sos();
      isFalled = true;
    };
  }

  if (isSOSPress()) {
    sos();
  }
  
  cnt_loop++;
  if (cnt_loop == 100) {
    cnt_loop = 0;
    // thêm điều kiện gậy để im, không hoạt động, dùng cảm biến gia tốc
    updateSubcriber(); // 20s update
  }
  delay(200);
}
