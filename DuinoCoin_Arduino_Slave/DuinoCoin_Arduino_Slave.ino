/*
  DoinoCoin_ArduinoSlave.ino
  created 10 05 2021
  by Luiz H. Cassettari
*/

void setup() {
  Serial.begin(115200);
  DuinoCoin_setup();
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  if (DuinoCoin_loop())
  {
    Serial.print("Job Done : ");
    Serial.print(DuinoCoin_response());
  }
  if (runEvery(250))
  {
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  }
}
