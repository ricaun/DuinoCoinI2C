/*
  DuinoCoin_Pool.ino
  created 31 07 2021
  by Luiz H. Cassettari
*/

#if ESP8266
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#endif
#if ESP32
#include <HTTPClient.h>
#endif

#include <ArduinoJson.h>

const char * urlPool = "http://51.15.127.80:4242/getPool";

void UpdateHostPort(String input)
{
  // {"ip":"server.duinocoin.com","port":2812,"name":"Main server"}
  DynamicJsonDocument doc(256);
  deserializeJson(doc, input);

  const char* name = doc["name"];
  const char* ip = doc["ip"];
  int port = doc["port"];

  Serial.println("[ ]Update " + String(name) + " " + String(ip) + " " + String(port));
  SetHostPort(String(ip), port);
}

void UpdatePool()
{
  String input = httpGetString(urlPool);
  if (input == "") return;
  UpdateHostPort(input);
}

String httpGetString(String URL)
{
  String payload = "";
  WiFiClient client;
  HTTPClient http;
  if (http.begin(client, URL))
  {
    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK)
    {
      payload = http.getString();
    }
    else
    {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
  }
  return payload;
}
