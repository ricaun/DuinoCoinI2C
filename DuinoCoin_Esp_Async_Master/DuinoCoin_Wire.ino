/*
  DuinoCoin_Wire.ino
  created 10 05 2021
  by Luiz H. Cassettari
*/

#include <Wire.h>

#if ESP8266
#define SDA 4 // D2 - A4 - GPIO4
#define SCL 5 // D1 - A5 - GPIO5
#endif

#if ESP32
#define SDA 21
#define SCL 22
#endif

#define WIRE_CLOCK 100000

void wire_setup()
{
  //pinMode(SDA, INPUT_PULLUP);
  //pinMode(SCL, INPUT_PULLUP);
  Wire.begin(SDA, SCL);
  Wire.setClock(WIRE_CLOCK);
  wire_readAll();
}

void wire_readAll()
{
  for (byte address = 1; address < 127; address++ )
  {
    if (wire_exists(address))
    {
      Serial.print("Address: ");
      Serial.println(address);
      wire_readLine(address);
    }
  }
}

boolean wire_exists(byte address)
{
  Wire.beginTransmission(address);
  byte error = Wire.endTransmission();
  return (error == 0);
}

void wire_sendJob(byte address, String lastblockhash, String newblockhash, int difficulty)
{
  String job = lastblockhash + "," + newblockhash + "," + difficulty;
  Wire_sendln(address, job);
}

void Wire_sendln(byte address, String message)
{
  Wire_send(address, message + "\n");
}

void Wire_send(byte address, String message)
{
  for (int i = 0; i < message.length(); i++)
  {
    Wire.beginTransmission(address);
    Wire.write(message.charAt(i));
    Wire.endTransmission();
  }
}

String wire_readLine(int address)
{
  wire_runEvery(0);
  char end = '\n';
  String str = "";
  boolean done = false;
  while (!done)
  {
    Wire.requestFrom(address, 1);
    if (Wire.available())
    {
      char c = Wire.read();
      //Serial.print(c);
      if (c == end)
      {
        break;
        done = true;
      }
      str += c;
    }
    if (wire_runEvery(2000)) break;
  }
  //str += end;
  return str;
}

boolean wire_runEvery(unsigned long interval)
{
  static unsigned long previousMillis = 0;
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval)
  {
    previousMillis = currentMillis;
    return true;
  }
  return false;
}
