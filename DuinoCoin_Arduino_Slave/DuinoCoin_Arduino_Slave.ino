/*
  DoinoCoin_Arduino_Slave.ino
  created 10 05 2021
  by Luiz H. Cassettari
*/

void setup() {
  Serial.begin(115200);
  DuinoCoin_setup();
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
}

void loop() {
  if (DuinoCoin_loop())
  {
    Blink();
  }
}

void Blink()
{
  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);
  digitalWrite(LED_BUILTIN, LOW);
}
