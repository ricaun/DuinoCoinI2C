/*
  DuinoCoin_Wire.ino
  created 10 05 2021
  by Luiz H. Cassettari
*/

#include <Wire.h>

#define SDA 4 // D2 - A4 - GPIO4
#define SCL 5 // D1 - A5 - GPIO5

void wire_setup()
{
  pinMode(SDA, INPUT_PULLUP);
  pinMode(SCL, INPUT_PULLUP);
  Wire.begin(SDA, SCL);
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
  }
  //str += end;
  return str;
}
