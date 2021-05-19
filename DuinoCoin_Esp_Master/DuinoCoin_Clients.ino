/*
  DuinoCoin_Clients.ino
  created 10 05 2021
  by Luiz H. Cassettari
*/

#if ESP8266
#include <ESP8266WiFi.h> // Include WiFi library
#include <ESP8266mDNS.h> // OTA libraries
#include <WiFiUdp.h>
#endif
#if ESP32
#include <WiFi.h>
#endif

#define CLIENTS 10

#define END_TOKEN  '\n'
#define SEP_TOKEN  ','

const char * host = "51.15.127.80"; // Static server IP
const int port = 2811;

WiFiClient clients[CLIENTS];
byte clientsWaitJob[CLIENTS];
int clientsShares[CLIENTS];
String clientsBuffer[CLIENTS];
unsigned long clientsTimes[CLIENTS];

bool clients_connected(byte i)
{
  wire_readLine(i);
  if (clients[i].connected())
  {
    return true;
  }
  Serial.print("[" + String(i) + "]");
  Serial.println("Connecting to Duino-Coin server...");
  if (!clients[i].connect(host, port))
  {
    Serial.print("[" + String(i) + "]");
    Serial.println("Connection failed.");
    return false;
  }
  clientsShares[i] = 0;
  clientsWaitJob[i] = 0;
  clientsTimes[i] = millis();
  clientsBuffer[i] = "";
  waitForClientData(clients[i]);
  return true;
}

void clients_loop()
{
  for (int i = 0; i < CLIENTS; i++)
  {
    if (wire_exists(i + 1) && clients_connected(i))
    {

      if (clientsWaitJob[i] == 0)
      {
        clients_requestJob(i);
      }

      if (clientsWaitJob[i] == 1)
      {
        clients_waitRequestJob(i);
      }

      if (clientsWaitJob[i] == 2)
      {
        clients_sendJobDone(i);
      }

      if (clientsWaitJob[i] == 3)
      {
        clients_waitFeedbackJobDone(i);
      }

    }
  }
}

void clients_requestJob(byte i)
{
  Serial.print("[" + String(i) + "]");
  Serial.println("Job Request: " + String(ducouser));
  clients[i].print("JOB," + String(ducouser) + "," + JOB);
  clientsWaitJob[i] = 1;
}

void clients_waitRequestJob(byte i)
{
  String clientBuffer = clients_readData(i);
  if (clientBuffer.length() > 0)
  {
    String hash = getValue(clientBuffer, SEP_TOKEN, 0);
    String job = getValue(clientBuffer, SEP_TOKEN, 1);
    unsigned int diff = getValue(clientBuffer, SEP_TOKEN, 2).toInt();

    Serial.print("[" + String(i) + "]");
    Serial.println("Job Receive: " + String(diff));
    
    wire_sendJob(i + 1, hash, job, diff);
    clientsWaitJob[i] = 2;
  }
}

void clients_sendJobDone(byte i)
{
  String responseJob = wire_readLine(i + 1);
  if (responseJob.length() > 0)
  {
    StreamString response;
    response.print(responseJob);

    int job = response.readStringUntil(',').toInt();
    int time = response.readStringUntil(',').toInt();
    String id = response.readStringUntil('\n');
    float HashRate = job / (time * .000001f);

    if (id.length() > 0) id = "," + id;

    String identifier = String(rigIdentifier)+ "-" + String(i);

    clients[i].print(String(job) + "," + String(HashRate) + "," + MINER + "," + String(identifier) + id);
    Serial.print("[" + String(i) + "]");
    Serial.println("Job Done: (" + String(job) + ")" + " Hashrate: " + String(HashRate));
    clientsWaitJob[i] = 3;
  }
}

void clients_waitFeedbackJobDone(byte i)
{
  String clientBuffer = clients_readData(i);
  if (clientBuffer.length() > 0)
  {
    unsigned long time = (millis() - clientsTimes[i]);
    clientsShares[i]++;
    int Shares = clientsShares[i];
    Serial.print("[" + String(i) + "]");
    Serial.println("Job " + clientBuffer  + ": Share #" + String(Shares) + " " + timeString(time));
    clientsWaitJob[i] = 0;
  }
}

String timeString(unsigned long t) {
  String str = "";
  t /= 1000;
  int s = t % 60;
  int m = (t / 60) % 60;
  int h = (t / 3600);
  str += h;
  str += ":";
  if (m < 10) str += "0";
  str += m;
  str += ":";
  if (s < 10) str += "0";
  str += s;
  return str;
}


String clients_readData(byte i)
{
  String str = "";
  while (clients[i].available()) {
    char c = clients[i].read();
    if (c == END_TOKEN)
    {
      str = clientsBuffer[i];
      clientsBuffer[i] = "";
    }
    else 
      clientsBuffer[i] += c;
  }
  return str;
}

// https://stackoverflow.com/questions/9072320/split-string-into-string-array
String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }

  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

String waitForClientData(WiFiClient & client) {
  String buffer;
  while (client.connected()) {
    if (client.available()) {
      buffer = client.readStringUntil(END_TOKEN);
      if (buffer.length() == 1 && buffer[0] == END_TOKEN)
        buffer = "???\n"; // NOTE: Should never happen...
      if (buffer.length() > 0)
        break;
    }
    handleSystemEvents();
  }
  return buffer;
}
