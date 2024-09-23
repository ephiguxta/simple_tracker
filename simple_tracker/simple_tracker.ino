#include <TinyGPSPlus.h>
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
  if (Serial1.available() > 0) {
    gps.encode(Serial1.read());
  }

  Serial.printf("(%d:%d:%d) lat: %.6f lon: %.6f\n",
                gps.time.hour(), gps.time.minute(), gps.time.second(),
                gps.location.lat(), gps.location.lng());

  delay(500);
}
