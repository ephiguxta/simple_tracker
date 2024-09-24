#include <TinyGPSPlus.h>
#include <string.h>
#include "BluetoothSerial.h"

BluetoothSerial SerialBT;
TinyGPSPlus gps;

const uint8_t rx = 16;
const uint8_t tx = 17;

void setup() {
  SerialBT.begin("telemetria");
  delay(32);

  uint8_t mac_addr[6];
  SerialBT.getBtAddress(mac_addr);

  Serial.begin(9600);
  delay(32);

  Serial1.begin(9600, SERIAL_8N1, rx, tx);
  delay(32);
}

void loop() {
  char msg[64] = { 0 };

  if (Serial1.available() > 0) {
    char data = Serial1.read();
    //Serial.printf("%c", data);
    gps.encode(data);
    delay(16);
  }

  snprintf(msg, 64, "(%d:%d:%d) lat: %.6f lon: %.6f\n",
           gps.time.hour(), gps.time.minute(), gps.time.second(),
           gps.location.lat(), gps.location.lng());

  uint16_t msg_size = strlen(msg);
  for (uint16_t i = 0; i < msg_size; i++) {
    SerialBT.write(msg[i]);
    delay(16);
  }
}
