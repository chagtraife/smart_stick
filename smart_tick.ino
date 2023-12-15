#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

#define MOTOR_PIN 13
#define EN_PIN 4
#define A1_PIN 3
#define A0_PIN 2

typedef enum {
  MPU6050,
  SR04T,
  GPS,
  SIM,
  DF,
  NO_DEVICE
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
}

void setup() {
  switch_uart(NO_DEVICE);
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
  Serial.print(a.acceleration.y);
  Serial.print(", Z: ");
  Serial.print(a.acceleration.z);
  Serial.println(" m/s^2");
  Serial.print("Rotation X: ");
  Serial.print(g.gyro.x);
  Serial.print(", Y: ");
  Serial.print(g.gyro.y);
  Serial.print(", Z: ");
  Serial.print(g.gyro.z);
  Serial.println(" rad/s");
  Serial.print("Temperature: ");
  Serial.print(temp.temperature);
  Serial.println(" degC");
  Serial.println("");
  delay(500);
}



void loop() {
  switch_uart(SR04T);
  

}
