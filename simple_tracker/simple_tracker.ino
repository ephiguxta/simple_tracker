#include <string.h>
#include "BluetoothSerial.h"

BluetoothSerial SerialBT;

//bool check_lat_lng();
bool valid_char(const char data);
uint8_t find_null_byte_pos(const char msg[128]);
bool line_has_lat_lng(const char msg[128]);
uint16_t line_checksum(const uint8_t null_byte_pos, const char msg[128]);
bool valid_checksum(const char msg[128]);

void setup() {
  SerialBT.begin("telemetria");
  delay(32);

  // TODO: utilizar isso como sufixo do nome do dispositivo (com salt)
  uint8_t mac_addr[6];
  SerialBT.getBtAddress(mac_addr);

  Serial.begin(9600);
  delay(32);
}

void loop() {
  char msg[128] = { 0 };

  if (Serial.available() > 0) {
    char data;

    for (uint8_t i = 0; i < 128; i++) {
      data = Serial.read();

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

  if (msg[0] != 0x00) {
    uint8_t null_byte_pos = find_null_byte_pos(msg);
    uint16_t checksum = line_checksum(null_byte_pos, msg);

    // 0x3030 == '00'
    if (checksum != 0x3030 && line_has_lat_lng(msg) && valid_checksum(msg)) {
    // if(checksum != 0x3030) {
      SerialBT.write((const uint8_t *) msg, null_byte_pos + 1);

      Serial.printf("\n(%s) ", msg);
      Serial.printf("%c%c ", ((checksum & 0xff00) >> 8), (checksum & 0x00ff));
      valid_checksum(msg);
    }
  }

  delay(25);
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

bool valid_checksum(const char msg[128]) {
  uint8_t null_byte_pos = find_null_byte_pos(msg);
  uint16_t possible_checksum = line_checksum(null_byte_pos, msg);

  uint16_t checksum = 0x0000;
  for(uint8_t i = 1; i < null_byte_pos - 3; i++) {
    checksum ^= msg[i];
  }

  uint8_t msb = (possible_checksum >> 8) - '0';
  uint8_t lsb = (possible_checksum & 0x00ff) - '0';
  if (lsb > 9) {
    lsb -= 1;
  }

  // FIXME: descomente o print abaixo e perceba que
  // quando o valor dor lsb é maior que 9 ele retorna dois
  // dígitos, por exemplo 'a' fica como 10.
  // então... valores com checksum válido mas com lsb maior
  // que 9 serão perdidos
  /*
  Serial.printf("[%x %x] (%x == %x) (%x == %x)\n",
    possible_checksum, checksum,
	 msb, ( (checksum & 0xf0) >> 4),
	 lsb, (checksum & 0x0f)
  );
  */

  if ( (msb == ( (checksum & 0xf0) >> 4)) && (lsb == (checksum & 0x0f)) ) {
    return true;
  }

  return false;
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

bool line_has_lat_lng(const char msg[128]) {
  char tag[6] = { 0 };

  for (uint8_t i = 1; i <= 5; i++) {
    tag[i - 1] = msg[i];
  }

  // TODO: GPGLL também é uma linha que contém lat e lng
  const char valid_tags[4][6] = {
    "GNRMC", "GPRMC", "GNGGA", "GPGGA"
  };

  for(uint8_t i = 0; i < 4; i++) {
    if(strncmp(valid_tags[i], tag, 5) == 0) {
      return true;
    }
  }

  return false;
}
