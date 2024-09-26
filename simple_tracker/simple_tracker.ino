#include <TinyGPSPlus.h>
#include <string.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <NetworkClientSecure.h>
#include "wifi.h"
#include "tg_credentials.h"

TinyGPSPlus gps;

const uint8_t rx = 16;
const uint8_t tx = 17;

bool check_lat_lng();
void connect_wifi();
bool send_data2tg(const char msg[64]);

void set_clock() {
  configTime(0, 0, "pool.ntp.org");

  Serial.print(F("Waiting for NTP time sync: "));
  time_t nowSecs = time(nullptr);
  while (nowSecs < 8 * 3600 * 2) {
    delay(500);
    Serial.print(F("."));
    yield();
    nowSecs = time(nullptr);
  }

  Serial.println();
  struct tm timeinfo;
  gmtime_r(&nowSecs, &timeinfo);
  Serial.print(F("Current time: "));
  Serial.print(asctime(&timeinfo));
}

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

  set_clock();
}

void loop() {
  char msg[64] = { 0 };

  if (Serial1.available() > 0) {
    char data = Serial1.read();
    //Serial.printf("%c", data);
    gps.encode(data);
    delay(16);
  }

  snprintf(msg, 64, "[%d:%d:%d] lat: %.6f lon: %.6f",
           gps.time.hour(), gps.time.minute(), gps.time.second(),
           gps.location.lat(), gps.location.lng());

  if (check_lat_lng()) {
    uint8_t msg_size = strlen(msg);
    Serial.printf("%s\n", msg);
    // SerialBT.write(msg, msg_size);
    send_data2tg(msg);
    delay(256);
  }
}

bool check_lat_lng() {
  // FIXME: inverta a lógica, atualmente está dessa forma apenas
  // para testes.
  if (gps.location.lat() == 0 || gps.location.lng() == 0) {
    return true;
  }

  return true;
}

bool send_data2tg(const char msg[64]) {
  char url[256] = { 0 };
  HTTPClient https;

  if(WiFi.status() == WL_CONNECTED) {
    delay(3000);
    NetworkClientSecure *client = new NetworkClientSecure;
    
    if(client) {
      client->setCACert(rootCACertificate);
      snprintf(url, 78, "%s/bot%s", endpoint, key);

      strcat(url, "/sendMessage");

      // montar o parâmetro de ID do chat 
      char chat_param[16] = "?chat_id=";
      strncat(chat_param, chat_id, strlen(chat_id));

      strcat(url, chat_param);

      char text[128] = "&text=";
      strcat(text, msg);
      strcat(url, text);

      Serial.printf("(%s)\n", url);

      if(https.begin(*client, url)) {

        int https_code = https.GET();

        if(https_code == HTTP_CODE_OK) {
          Serial.printf("Mensagem Ok!");
        } else {
          Serial.printf("%s\n", https.errorToString(https_code).c_str());
        }

        delay(128);
        https.end();
      }

    } else {
      Serial.printf("O WiFi não está conectado, reiniciando o ESP32\n");
      delay(100);
      ESP.restart();
    }

    delay(1e3 * 60);
   
    delete client;
  }

  return true;
}
