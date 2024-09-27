void setup() {
  Serial.begin(9600);
  Serial3.begin(9600);
}

void loop() {
  int n_bytes = Serial3.available();

  if(n_bytes > 0) {
    for(int i = 0; i < n_bytes; i++) {
      char data = Serial3.read();
      if (data == '\r') {
        continue;
      }
      Serial.print(data);
      delay(16);
    }
  }
}
