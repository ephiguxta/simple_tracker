#include <string.h>
#include "BluetoothSerial.h"

BluetoothSerial SerialBT;

const uint8_t rx = 16;
const uint8_t tx = 17;

//bool check_lat_lng();
bool valid_char(char data);

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
  Serial1.setTimeout(1000);

  if (Serial1.available() > 0) {
    char data;

    for (uint8_t i = 0; i < 128; i++) {
      data = Serial1.read();

      // se o dado não for um caractere válido e não iniciar com '$'
      if ((i == 0 && data != '$') || !valid_char(data)) {
        msg[i] = '\0';
        break;
      }

      msg[i] = data;
      delay(0.2);
    }
  }

  Serial.printf("%s\n", msg);
  delay(0.5);

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

bool valid_char(char data) {
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
/*
bool check_lat_lng() {
  if (gps.location.lat() == 0 || gps.location.lng() == 0) {
    return false;
  }

  return true;
}
*/
