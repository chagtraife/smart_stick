#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

#define MOTOR_PIN 11
#define EN_PIN 4
#define A1_PIN 3
#define A0_PIN 2
#define BUTTON_PIN 10

#define isButtonPress (digitalRead(BUTTON_PIN) == LOW)

unsigned int distance = 0;

typedef enum {
  MPU6050,
  SR04T,
  GPS,
  SIM,
  DF,
  USB
} module_t;

Adafruit_MPU6050 mpu;

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

void setup() {
  pinMode(EN_PIN, OUTPUT);
  pinMode(A1_PIN, OUTPUT); 
  pinMode(A0_PIN, OUTPUT); 
  pinMode(BUTTON_PIN, INPUT);

  switch_uart(USB);
  Serial.begin(9600);
  Serial.println("hello");

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

  switch_uart(SR04T);
  Serial.write(0x55);
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
      }
      // switch_uart(USB);
      // // Print to serial monitor
      // Serial.print("distance: ");
      // Serial.println(distance);
      // Serial.println(" mm");
    }
  }
}

void read_GPS() {
  switch_uart(GPS);

}

void genWarning() {}

void sendSMS() {
  switch_uart(SIM);

}

void askUser() {
  switch_uart(DF);
}

bool checkResponse() {
  int cnt = 0;
  while((!isButtonPress)) {
    cnt++;
    delay(50);
    if (cnt > 50) {
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

bool isFall() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  Serial.print(a.acceleration.y); // dung gia tri nay de kiểm tra xem có ngã hay không, gần = 0 thì ngã
  if (abs(a.acceleration.y) < 1) return true;
  else return false;
}

void loop() {
  read_SR04T();
  if (distance < 2000) {
    if (distance < 200) distance = 200;
    distance -= 200;
    //rung theo distance
    int cycle = 255 - (distance * 255) / 1800;
    analogWrite(MOTOR_PIN, cycle);
  }
  
  if (isFall()) {
    askUser();
    if (!checkResponse()) {
      read_GPS();
      sendSMS();
      genWarning();
    }
  }
  if (isSOSPress()) {
      read_GPS();
      sendSMS();
      genWarning();
  }
  delay(20);
}
