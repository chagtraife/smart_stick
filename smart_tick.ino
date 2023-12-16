#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <TinyGPS.h>

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

const int PHONEBOOK_SIZE = 100; 

Adafruit_MPU6050 mpu;
TinyGPS gps;
String* phone_numbers;
int own_num = 0;

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
void alarm() {

}

void updateSubcriber() {
  String res = readSMS(1);
  if (res.indexOf("ERROR") != -1) return;
  String number;
  if (res.indexOf("register") != -1) {
    if (findPhoneNumber(number) == -1) {
      int idx = findEmptyPhonebookIndex();
      if (idx != -1) {
        addToPhonebook("sub", number, idx);
      }
    }
  } else if (res.indexOf("reset") != -1) {
    int idx = findPhoneNumber(number);
    if (idx != -1) {
      deletePhonebook(idx);
    }
  }
  deleteSMS(1);
}
bool addToPhonebook(String name, String phoneNumber, int index) {
  switch_uart(SIM);
  String command = "AT+CPBW=" + String(index) + ",\"" + phoneNumber + "\",129,\"" + name + "\"";
  Serial.println(command);
  delay(1000);

  // Đọc dữ liệu từ module SIM800L
  String response = "";
  while (Serial.available()) {
    response = Serial.readString();
    delay(10);
  }

  // Kiểm tra kết quả phản hồi để xác định xem số điện thoại đã được thêm vào danh bạ hay không
  if (response.indexOf("OK") != -1) {
    return true; // Số điện thoại đã được thêm vào danh bạ thành công
  } else {
    return false; // Thêm số điện thoại vào danh bạ không thành công
  }
}
bool deletePhonebook(int index) {
  switch_uart(SIM);
  String command = "AT+CPBW=" + String(index);
  command += "\r\n";
  Serial.println(command);
  delay(1000);

  // Đọc dữ liệu từ module SIM800L
  String response = "";
  while (Serial.available()) {
    response = Serial.readString();
    delay(10);
  }

  // Kiểm tra kết quả phản hồi để xác định xem số điện thoại đã được xóa hay không
  if (response.indexOf("OK") != -1) {
    return true; // Số điện thoại đã được xóa khỏi danh bạ thành công
  } else {
    return false; // Xóa số điện thoại không thành công hoặc có lỗi xảy ra
  }
}
int findEmptyPhonebookIndex() {
  for (int i = 1; i <= PHONEBOOK_SIZE; ++i) {
    if (!checkPhonebook(i)) {
      return i;
    }
  }
  return -1; // Không tìm thấy vị trí trống trong danh bạ
}
int findPhoneNumber(String phoneNumber) {
  switch_uart(SIM);
  String command = "AT+CPBF=\"" + phoneNumber + "\"";
  Serial.println(command);
  delay(1000);

  // Đọc dữ liệu từ module SIM800L
  String response = "";
  while (Serial.available()) {
    response = Serial.readString();
    delay(10);
  }

  // Kiểm tra phản hồi để trích xuất vị trí từ dữ liệu phản hồi
  int index = -1;
  if (response.indexOf("+CPBF:") != -1) {
    int colonIndex = response.indexOf(":");
    int commaIndex = response.indexOf(",");
    if (commaIndex != -1) {
      index = response.substring(colonIndex + 1, commaIndex).toInt();
    }
  }

  return index;
}
String getContactInfo(int index) {
  switch_uart(SIM);
  String command = "AT+CPBR=" + String(index);
  Serial.println(command);
  delay(1000);

  // Đọc dữ liệu từ module SIM800L
  String response = "";
  while (Serial.available()) {
    response = Serial.readString();
    delay(10);
  }

  // Xử lý phản hồi để trích xuất thông tin liên hệ từ dữ liệu phản hồi
  if (response.indexOf("+CPBR:") != -1) {
    int colonIndex = response.indexOf(":");
    int quotationIndex1 = response.indexOf("\"");
    int quotationIndex2 = response.indexOf("\"", quotationIndex1 + 1);

    if (quotationIndex1 != -1 && quotationIndex2 != -1) {
      String contactInfo = response.substring(quotationIndex1 + 1, quotationIndex2);
      return contactInfo;
    }
  }

  return ""; // Trả về chuỗi rỗng nếu không có thông tin liên hệ tại vị trí đó
}
bool checkPhonebook(int index) {
  switch_uart(SIM);
  String command = "AT+CPBR=" + String(index);
  Serial.println(command);
  delay(1000);

  // Đọc dữ liệu từ module SIM800L
  String response = "";
  while (Serial.available()) {
    response = Serial.readString();
    delay(10);
  }

  // Kiểm tra phản hồi để xác định xem vị trí trong danh bạ có dữ liệu hay không
  if (response.indexOf("ERROR") == -1 && response.indexOf("OK") != -1) {
    return true; // Vị trí trong danh bạ có dữ liệu
  } else {
    return false; // Vị trí trong danh bạ không có dữ liệu hoặc có lỗi xảy ra
  }
}
bool deleteSMS(int index) {
  switch_uart(SIM);
  String command = "AT+CMGD=";
  command += index; // Truyền số thứ tự tin nhắn cần xóa
  command += "\r";
  Serial.print(command);
  delay(3000);

  // Đọc dữ liệu từ module SIM800L
  String response = "";
  while (Serial.available()) {
    response = Serial.readString();
    delay(10);
  }

  // Kiểm tra kết quả phản hồi để xác định xem tin nhắn đã được xóa hay không
  if (response.indexOf("OK") != -1) {
    return true; // Tin nhắn đã được xóa thành công
  } else {
    return false; // Xóa tin nhắn không thành công hoặc không có tin nhắn tại vị trí đó
  }
}
String readSMS(int index) {
  switch_uart(SIM);
  String command = "AT+CMGF=1\r"; // Đặt chế độ tin nhắn văn bản
  Serial.print(command);
  delay(1000);

  command = "AT+CMGR=";
  command += index; // Truyền số thứ tự tin nhắn cần đọc
  command += "\r";
  Serial.print(command);
  delay(3000);

  // Đọc dữ liệu từ module SIM800L
  String response = "";
  while (Serial.available()) {
    response = Serial.readString();
    delay(10);
  }

  return response;
}
void sendSMS(String number, String message) {
  switch_uart(SIM);
  Serial.println("AT+CMGF=1"); // Đặt chế độ văn bản
  delay(100);
  Serial.println("AT+CMGS=\"" + number + "\""); // Gửi tới số điện thoại cụ thể
  delay(100);
  Serial.println(message); // Nội dung tin nhắn
  delay(100);
  Serial.println((char)26); // CTRL+Z kết thúc tin nhắn
  delay(100);
  Serial.println();
}
void askUser() {
  switch_uart(DF);
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

void loop() {
  read_SR04T();

  if (isFall()) {
    askUser();
    if (!checkResponse()) {
      sos();
    }
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
void waitForSerial(unsigned long timeout_millis) {
  unsigned long start = millis();
  while (!Serial) {
    if (millis() - start > timeout_millis)
      break;
  }
}
