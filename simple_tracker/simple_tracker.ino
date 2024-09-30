#include <string.h>
#include "BluetoothSerial.h"

BluetoothSerial SerialBT;

const uint8_t rx = 16;
const uint8_t tx = 17;

//bool check_lat_lng();
bool valid_char(const char data);
uint8_t find_null_byte_pos(const char msg[128]);
bool is_gnrmc_line(const char msg[128]);
uint16_t line_checksum(const uint8_t null_byte_pos, const char msg[128]);

void setup() {
  SerialBT.begin("telemetria");
  delay(32);

  // TODO: utilizar isso como sufixo do nome do dispositivo (com salt)
  uint8_t mac_addr[6];
  SerialBT.getBtAddress(mac_addr);

  Serial.begin(9600);
  delay(32);

  Serial1.begin(9600, SERIAL_8N1, rx, tx);
  delay(32);
}

void loop() {
  char msg[128] = { 0 };

  if (Serial1.available() > 0) {
    char data;

    for (uint8_t i = 0; i < 128; i++) {
      data = Serial1.read();

      if (data == '\0') {
        msg[i] = data;
        break;
      }

      // se o dado não for um caractere válido e não iniciar com '$'
      if ((i == 0 && data != '$') || !valid_char(data)) {
        msg[i] = '\0';
        break;
      }

      msg[i] = data;
      delay(0.2);
    }
  }

  // Serial.printf("%s\n", msg);
  if (msg[0] != 0x00) {
    uint8_t null_byte_pos = find_null_byte_pos(msg);
    uint16_t checksum = line_checksum(null_byte_pos, msg);

    // 0x3030 == '00'
    if (checksum != 0x3030 && is_gnrmc_line(msg)) {
      Serial.printf("(%s) ", msg);
      Serial.printf("%c%c\n", ((checksum & 0xff00) >> 8), (checksum & 0x00ff));
    }
  }

  delay(25);

  /*
  snprintf((char *)msg, 64, "(%d:%d:%d) lat: %.6f lon: %.6f\n",
           gps.time.hour(), gps.time.minute(), gps.time.second(),
           gps.location.lat(), gps.location.lng());

  if (check_lat_lng()) {
    uint8_t msg_size = strlen((const char *)msg);
    SerialBT.write(msg, msg_size);
    delay(256);
  }
  */
}

bool valid_char(const char data) {
  if (data >= 'A' && data <= 'Z') {
    return true;
  }

  if (data >= '0' && data <= '9') {
    return true;
  }

  if (data == '*' || data == '.' || data == ',' || data == '$') {
    return true;
  }

  return false;
}

uint8_t find_null_byte_pos(const char msg[128]) {
  uint8_t null_byte_pos = 0;

  for (uint8_t i = 0; i < 128; i++) {
    if (msg[i] == 0x00) {
      null_byte_pos = i;
      break;
    }
  }

  return null_byte_pos;
}

uint16_t line_checksum(const uint8_t null_byte_pos, const char msg[128]) {
  // 0x30 == '0'
  uint16_t checksum = 0x3030;

  if (null_byte_pos > 6) {
    char data = msg[null_byte_pos - 3];

    if (data == '*') {
      uint8_t msb = msg[null_byte_pos - 2];
      uint8_t lsb = msg[null_byte_pos - 1];

      // filtro pra ordenar os bytes e caber num uint16_t
      checksum = 0x00ff & lsb;
      checksum |= (0xff00 & (msb << 8));
    }

    return checksum;
  }

  return checksum;
}

bool is_gnrmc_line(const char msg[128]) {
  char tag[6] = { 0 };
  tag[5] = '\0';

  for (uint8_t i = 1; i <= 5; i++) {
    tag[i - 1] = msg[i];
  }

  const char *gnrmc_tag = "GNRMC";
  bool is_gnrmc = true;
  for (uint8_t i = 0; i < 5; i++) {
    if (tag[i] != gnrmc_tag[i]) {
      is_gnrmc = false;
    }
  }

  return is_gnrmc;
}

/*
bool check_lat_lng() {
  if (gps.location.lat() == 0 || gps.location.lng() == 0) {
    return false;
  }

  return true;
}
*/
