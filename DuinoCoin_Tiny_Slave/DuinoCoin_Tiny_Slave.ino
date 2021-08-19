/*
  DoinoCoin_Tiny_Slave.ino
  created 07 08 2021
  by Luiz H. Cassettari
*/

#include <ArduinoUniqueID.h>  // https://github.com/ricaun/ArduinoUniqueID
#include <EEPROM.h>
#include <Wire.h>
#include "sha1.h"

#if defined(ARDUINO_AVR_UNO) | defined(ARDUINO_AVR_PRO)
#define SERIAL_LOGGER Serial
#define LED LED_BUILTIN
#endif

// ATtiny85 - http://drazzy.com/package_drazzy.com_index.json
// SCL - PB2 - 2
// SDA - PB0 - 0

//#define FIND_I2C

#define ADDRESS_I2C 1
#define EEPROM_ADDRESS 0

#ifdef SERIAL_LOGGER
#define SerialBegin()              SERIAL_LOGGER.begin(115200);
#define SerialPrint(x)             SERIAL_LOGGER.print(x);
#define SerialPrintln(x)           SERIAL_LOGGER.println(x);
#else
#define SerialBegin()
#define SerialPrint(x)
#define SerialPrintln(x)
#endif

#ifdef LED
#define LedBegin()                pinMode(LED, OUTPUT);
#define LedHigh()                 digitalWrite(LED, HIGH);
#define LedLow()                  digitalWrite(LED, LOW);
#define LedBlink()                LedHigh(); delay(100); LedLow(); delay(100);
#else
#define LedBegin()
#define LedHigh()
#define LedLow()
#define LedBlink()
#endif

#define BUFFER_MAX 88
#define HASH_BUFFER_SIZE 20
#define CHAR_END '\n'
#define CHAR_DOT ','

static const char DUCOID[] PROGMEM = "DUCOID";
static const char ZEROS[] PROGMEM = "000";

static byte address;
static char buffer[BUFFER_MAX];
static uint8_t buffer_position;
static uint8_t buffer_length;
static bool working;
static bool jobdone;


void(* resetFunc) (void) = 0;//declare reset function at address 0

// --------------------------------------------------------------------- //
// setup
// --------------------------------------------------------------------- //

void setup() {
  SerialBegin();
  initialize_i2c();
  LedBegin();
  LedBlink();
  LedBlink();
  LedBlink();
}

// --------------------------------------------------------------------- //
// loop
// --------------------------------------------------------------------- //

void loop() {
  do_work();
  millis(); // ????? For some reason need this to work the i2c
#ifdef SERIAL_LOGGER
  if (SERIAL_LOGGER.available())
  {
#ifdef EEPROM_ADDRESS
    EEPROM.write(EEPROM_ADDRESS, SERIAL_LOGGER.parseInt());
#endif
    resetFunc();
  }
#endif
}

// --------------------------------------------------------------------- //
// run
// --------------------------------------------------------------------- //

boolean runEvery(unsigned long interval)
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

// --------------------------------------------------------------------- //
// work
// --------------------------------------------------------------------- //

void do_work()
{
  if (working)
  {
    LedHigh();
    SerialPrintln(buffer);

    if (buffer[0] == '#')
    {
      resetFunc();
    }

#ifdef FIND_I2C
    if (buffer[0] == '@')
    {
      address = find_i2c();
      memset(buffer, 0, sizeof(buffer));
      buffer_position = 0;
      buffer_length = 0;
      working = false;
      jobdone = false;
      return;
    }
#endif

    do_job();
  }
  LedLow();
}

void do_job()
{
  unsigned long startTime = millis();
  int job = work();
  unsigned long endTime = millis();
  unsigned int elapsedTime = endTime - startTime;
  memset(buffer, 0, sizeof(buffer));
  char cstr[16];

  // Job
  itoa(job, cstr, 10);
  strcpy(buffer, cstr);
  buffer[strlen(buffer)] = CHAR_DOT;

  // Time
  itoa(elapsedTime, cstr, 10);
  strcpy(buffer + strlen(buffer), cstr);
  strcpy_P(buffer + strlen(buffer), ZEROS);
  buffer[strlen(buffer)] = CHAR_DOT;

  // DUCOID
  strcpy_P(buffer + strlen(buffer), DUCOID);
  for (size_t i = 0; i < 8; i++)
  {
    itoa(UniqueID8[i], cstr, 16);
    if (UniqueID8[i] < 16) strcpy(buffer + strlen(buffer), "0");
    strcpy(buffer + strlen(buffer), cstr);
  }

  SerialPrintln(buffer);

  buffer_position = 0;
  buffer_length = strlen(buffer);
  working = false;
  jobdone = true;
}

int work()
{
  char delimiters[] = ",";
  char *lastHash = strtok(buffer, delimiters);
  char *newHash = strtok(NULL, delimiters);
  char *diff = strtok(NULL, delimiters);
  buffer_length = 0;
  buffer_position = 0;
  return work(lastHash, newHash, atoi(diff));
}

//#define HTOI(c) ((c<='9')?(c-'0'):((c<='F')?(c-'A'+10):((c<='f')?(c-'a'+10):(0))))
#define HTOI(c) ((c<='9')?(c-'0'):((c<='f')?(c-'a'+10):(0)))
#define TWO_HTOI(h, l) ((HTOI(h) << 4) + HTOI(l))
//byte HTOI(char c) {return ((c<='9')?(c-'0'):((c<='f')?(c-'a'+10):(0)));}
//byte TWO_HTOI(char h, char l) {return ((HTOI(h) << 4) + HTOI(l));}

void HEX_TO_BYTE(char * address, char * hex, int len)
{
  for (int i = 0; i < len; i++) address[i] = TWO_HTOI(hex[2 * i], hex[2 * i + 1]);
}

// DUCO-S1A hasher
uint32_t work(char * lastblockhash, char * newblockhash, int difficulty)
{
  if (difficulty > 655) return 0;
  HEX_TO_BYTE(newblockhash, newblockhash, HASH_BUFFER_SIZE);
  for (int ducos1res = 0; ducos1res < difficulty * 100 + 1; ducos1res++)
  {
    Sha1.init();
    Sha1.print(lastblockhash);
    Sha1.print(ducos1res);
    if (memcmp(Sha1.result(), newblockhash, HASH_BUFFER_SIZE) == 0)
    {
      return ducos1res;
    }
  }
  return 0;
}

// --------------------------------------------------------------------- //
// i2c
// --------------------------------------------------------------------- //

void initialize_i2c(void) {
  address = ADDRESS_I2C;

#ifdef EEPROM_ADDRESS
  address = EEPROM.read(EEPROM_ADDRESS);
  if (address == 0 || address > 127) {
    address = ADDRESS_I2C;
    EEPROM.write(EEPROM_ADDRESS, address);
  }
#endif

  SerialPrint("Wire begin ");
  SerialPrintln(address);
  Wire.begin(address);
  Wire.onReceive(onReceiveJob);
  Wire.onRequest(onRequestResult);
}

void onReceiveJob(int howMany) {
  if (howMany == 0) return;
  if (working) return;
  if (jobdone) return;

  while (Wire.available()) {
    char c = Wire.read();
    buffer[buffer_length++] = c;
    if (buffer_length == BUFFER_MAX) buffer_length--;
    if (c == CHAR_END)
    {
      working = true;
    }
  }
}

void onRequestResult() {
  char c = CHAR_END;
  if (jobdone) {
    c = buffer[buffer_position++];
    if ( buffer_position == buffer_length) {
      jobdone = false;
      buffer_position = 0;
      buffer_length = 0;
    }
  }
  Wire.write(c);
}

// --------------------------------------------------------------------- //
// find_i2c
// --------------------------------------------------------------------- //

#ifdef FIND_I2C

byte find_i2c()
{
  unsigned long time = (unsigned long) getTrueRotateRandomByte() * 1000 + (unsigned long) getTrueRotateRandomByte();
  delayMicroseconds(time);
  Wire.begin();
  int address;
  for (address = 1; address < 127; address++ )
  {
    Wire.beginTransmission(address);
    int error = Wire.endTransmission();
    if (error != 0)
    {
      break;
    }
  }
  Wire.begin(address);
  //Wire.end();
  return address;
}

// ---------------------------------------------------------------
// True Random Numbers
// https://gist.github.com/bloc97/b55f684d17edd8f50df8e918cbc00f94
// ---------------------------------------------------------------

#if defined(ARDUINO_AVR_PRO)
#define ANALOG_RANDOM A6
#else
#define ANALOG_RANDOM A1
#endif

const int waitTime = 16;

byte lastByte = 0;
byte leftStack = 0;
byte rightStack = 0;

byte rotate(byte b, int r) {
  return (b << r) | (b >> (8 - r));
}

void pushLeftStack(byte bitToPush) {
  leftStack = (leftStack << 1) ^ bitToPush ^ leftStack;
}

void pushRightStackRight(byte bitToPush) {
  rightStack = (rightStack >> 1) ^ (bitToPush << 7) ^ rightStack;
}

byte getTrueRotateRandomByte() {
  byte finalByte = 0;

  byte lastStack = leftStack ^ rightStack;

  for (int i = 0; i < 4; i++) {
    delayMicroseconds(waitTime);
    int leftBits = analogRead(ANALOG_RANDOM);

    delayMicroseconds(waitTime);
    int rightBits = analogRead(ANALOG_RANDOM);

    finalByte ^= rotate(leftBits, i);
    finalByte ^= rotate(rightBits, 7 - i);

    for (int j = 0; j < 8; j++) {
      byte leftBit = (leftBits >> j) & 1;
      byte rightBit = (rightBits >> j) & 1;

      if (leftBit != rightBit) {
        if (lastStack % 2 == 0) {
          pushLeftStack(leftBit);
        } else {
          pushRightStackRight(leftBit);
        }
      }
    }

  }
  lastByte ^= (lastByte >> 3) ^ (lastByte << 5) ^ (lastByte >> 4);
  lastByte ^= finalByte;

  return lastByte ^ leftStack ^ rightStack;
}

#endif
