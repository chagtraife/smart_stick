#include "sim800.h"

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

