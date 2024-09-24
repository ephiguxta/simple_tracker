#include <TinyGPSPlus.h>
#include <string.h>
#include <WiFi.h>
#include "wifi.h"

TinyGPSPlus gps;

const uint8_t rx = 16;
const uint8_t tx = 17;

bool check_lat_lng();

void setup() {
  Serial.begin(9600);
  delay(32);

  delay(2000);

  WiFi.enableSTA(true);
  WiFi.begin(ssid, passwd);
  delay(512);

  while(WiFi.status() != WL_CONNECTED) {
    Serial.printf("Esperando a conexão WiFi!\n");
    delay(500);
  }

  Serial.printf("Conectou, meu IP é: %s\n",
      WiFi.localIP().toString().c_str());

  Serial1.begin(9600, SERIAL_8N1, rx, tx);
  delay(32);
}

void loop() {
  uint8_t msg[64] = { 0 };

  if (Serial1.available() > 0) {
    char data = Serial1.read();
    //Serial.printf("%c", data);
    gps.encode(data);
    delay(16);
  }

  snprintf((char *)msg, 64, "(%d:%d:%d) lat: %.6f lon: %.6f\n",
           gps.time.hour(), gps.time.minute(), gps.time.second(),
           gps.location.lat(), gps.location.lng());

  if (check_lat_lng()) {
    uint8_t msg_size = strlen((const char *)msg);
    Serial.printf("%s", msg);
    // SerialBT.write(msg, msg_size);
    delay(256);
  }
}

bool check_lat_lng() {
  // FIXME: inverta a lógica, atualmente está dessa forma apenas
  // para testes.
  if (gps.location.lat() == 0 || gps.location.lng() == 0) {
    return true;
  }

  return false;
}
