#include <string.h>
#include "BluetoothSerial.h"
#include "FS.h"
#include "SD_MMC.h"

#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

BluetoothSerial SerialBT;

bool valid_char(const char data);
uint8_t find_null_byte_pos(const char msg[128]);
// void get_lat_lng(const char msg[128], int tag_num);
uint16_t line_checksum(const uint8_t null_byte_pos, const char msg[128]);
int tag_id(const char msg[128]);
bool valid_checksum(const char msg[128]);

// TODO: faça essa variável existir apenas nas funções que precisam dela
// todas as chamadas terão ela como parâmetro
static char tag[6] = { 0 };

unsigned long my_time = 0;

// armazena a última mensagem do gps para fazer uma comparação com a
// mensagem seguinte
char old_msg[128] = { 0 };

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  SerialBT.begin("test");
  delay(32);

  // TODO: utilizar isso como sufixo do nome do dispositivo (com salt)
  uint8_t mac_addr[6];
  SerialBT.getBtAddress(mac_addr);

  Serial.begin(9600);
  delay(100);

  if(!SD_MMC.begin()) {
    delay(1000);
    Serial.printf("Montagem do cartão SD falhou!\n");
  }
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

    int tag = 0;
    tag = tag_id(msg);

    // 0x3030 == '00'
    if (checksum != 0x3030 && valid_checksum(msg) && tag) {
      Serial.printf("[%ld] (%s)\n", millis(), msg);

      msg[null_byte_pos] = '\r';
      msg[null_byte_pos + 1] = '\n';

      // caso a nova linha gerada pelo gps for igual a anterior,
      // não enviamos ao cliente.
      if(strncmp(old_msg, msg, 128) != 0) {
        for(int i = 0; i < 128; i++) {
          old_msg[i] = msg[i];
        }

        SerialBT.write((const uint8_t *) msg, find_null_byte_pos(msg));
      }

      // salvando os dados a cada 2min no cartão sd
      // 120e3 ms == 2min
      if((millis() - my_time) >= 120000) {
        my_time = millis();
        File file = SD_MMC.open("/gps_log.txt", FILE_APPEND);
        // TODO: tratar para caso isso falhe
        file.print(msg);
        delay(100);
        file.close();
      }
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

  // Essa subtração serve para "burlar" um bug que temos quando
  // o lsb fica maior que 9, por exemplo se for 'a' ele retorna 11
  // O -7 funcionou e não faço ideia do porquê :)
  if (lsb > 9) {
    lsb -= 7;
  }

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

int tag_id(const char msg[128]) {
  char msg_tag[6] = { 0 };

  for (uint8_t i = 1; i <= 5; i++) {
    msg_tag[i - 1] = msg[i];
    tag[i - 1] = msg[i];
  }

  const char valid_tags[6][6] = {
    "GNGGA", "GPGGA", "GNRMC", "GPRMC", "GPGLL", "GPZDA"
  };

  for(uint8_t i = 0; i < 4; i++) {
    if(strncmp(valid_tags[i], msg_tag, 6) == 0) {
      return i + 1;
    }
  }

  return 0;
}
