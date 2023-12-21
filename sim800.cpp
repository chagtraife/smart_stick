#include "sim800.h"

void waitSIM800L(uint16_t timeout) {
  uint16_t cnt = 0;
  while ((!SIM800L.available()) && (cnt < timeout)) {
    cnt++;
    delay(2);
  };
}

// Hàm đọc phản hồi từ UART
String getResponse() {
  String response = "";
  int cnt = 0;
  while ((!SIM800L.available()) && (cnt < 100)) {
    cnt++;
    delay(100);
  };
  while (SIM800L.available()) {
    char c = SIM800L.read();
    response += c;
    delay(2);
  }
  return response;
}

String sendCMD(String cmd) {
  switch_uart(SIM);
  SIM800L.println(cmd);
  String response = getResponse();
  if (response == "") response = "NO response";
  printDebug(response);
  return response;
}

void updateSubcriber() {
  // printDebug("updateSubcriber");
  String res = readSMS(1);
  if (res.indexOf("ERROR") != -1) return;
  String number;
  if (res.indexOf("CHU") != -1) {
    printDebug("===========");
    int idx_1 = res.indexOf("+84");
    printDebug("idx_1: " + String(idx_1));
    int idx_2 = res.indexOf(",", idx_1);
    printDebug("idx_2: " + String(idx_2));
    number = res.substring(idx_1, idx_2 - 1);
    printDebug("number: " + number);
    // if (findPhoneNumber(number) == -1) {
  //     int idx = findEmptyPhonebookIndex();
  //     if (idx != -1) {
  //       addToPhonebook("sub", number, idx);
  //     }
  //   }
  // } else if (res.indexOf("reset") != -1) {
  //   int idx = findPhoneNumber(number);
  //   if (idx != -1) {
  //     deletePhonebook(idx);
  //   }
  }
  // deleteSMS(1);
}

bool addToPhonebook(String name, String phoneNumber, int index) {
  switch_uart(SIM);
  String command = "AT+CPBW=" + String(index) + ",\"" + phoneNumber + "\",129,\"" + name + "\"";
  SIM800L.println(command);
  delay(1000);

  // Đọc dữ liệu từ module SIM800L
  String response = "";
  while (SIM800L.available()) {
    response = SIM800L.readString();
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
  SIM800L.println(command);
  delay(1000);

  // Đọc dữ liệu từ module SIM800L
  String response = "";
  while (SIM800L.available()) {
    response = SIM800L.readString();
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
  SIM800L.println(command);
  delay(1000);

  // Đọc dữ liệu từ module SIM800L
  String response = "";
  while (SIM800L.available()) {
    response = SIM800L.readString();
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
  SIM800L.println(command);
  delay(1000);

  // Đọc dữ liệu từ module SIM800L
  String response = "";
  while (SIM800L.available()) {
    response = SIM800L.readString();
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
  SIM800L.println(command);
  delay(1000);

  // Đọc dữ liệu từ module SIM800L
  String response = "";
  while (SIM800L.available()) {
    response = SIM800L.readString();
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
  String command = "AT+CMGD=";
  command += index; // Truyền số thứ tự tin nhắn cần xóa
  command += "\r";
  String response = sendCMD(command);

  // Kiểm tra kết quả phản hồi để xác định xem tin nhắn đã được xóa hay không
  if (response.indexOf("OK") != -1) {
    return true; // Tin nhắn đã được xóa thành công
  } else {
    return false; // Xóa tin nhắn không thành công hoặc không có tin nhắn tại vị trí đó
  }
}
String readSMS(int index) {
  sendCMD("AT");
  delay(1000);
  String command = "AT+CMGF=1\r"; // Đặt chế độ tin nhắn văn bản
  sendCMD(command);
  delay(1000);
  command = "AT+CMGR=";
  command += index; // Truyền số thứ tự tin nhắn cần đọc
  command += "\r";
  String response = sendCMD(command);
  return response;
}

void sendSMS(String number, String message) {
  // printDebug("sendSMS");
  sendCMD("AT");
  delay(1000);
  sendCMD("AT+CMGF=1");
  delay(1000);
  sendCMD("AT+CMGS=\"" + number + "\"\r\n");
  delay(1000);
  switch_uart(SIM);
  delay(1000);
  SIM800L.println(message); // Nội dung tin nhắn
  delay(100);
  SIM800L.println((char)26); // CTRL+Z kết thúc tin nhắn
  delay(1000);
  String response = getResponse();
  if (response == "") response = "NO response";
  printDebug(response);
}

