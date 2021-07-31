# DuinoCoinI2C

This project design to mine [Duino-Coin](https://github.com/revoxhere/duino-coin) using an Esp8266/Esp32 as a master and Arduino as a slave. 

Using the I2C communication to connect all the boards and make a scalable communication between the master and the slaves.

## Version

DuinoCoinI2C Version 2.6

# Arduino - Slave

All Slaves have the same code and should select the I2C Address automatically.

## Library Dependency

* [DuinoCoin](https://github.com/ricaun/arduino-DuinoCoin) (Handle the `Ducos1a` hash work)
* [ArduinoUniqueID](https://github.com/ricaun/ArduinoUniqueID) (Handle the chip ID)
* [StreamJoin](https://github.com/ricaun/StreamJoin) (StreamString for AVR)

## Automatic I2C Address 

The I2C Address on the Arduino is automatically updated when the board starts, if an Address already exists on the I2C bus the code finds another Address to use.

# Esp8266/Esp32 - Master

The master requests the job on the `DuinoCoin` server and sends the work to the slave (Arduino).

After the job is done, the slave sends back the response to the master (Esp8266/Esp32) and then sends back to the `DuinoCoin` server.

## Library Dependency

* [ArduinoJson](https://github.com/bblanchon/ArduinoJson) (Request Pool Version 2.6)
* [ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServerD) (ESPAsyncWebServer)
    * [ESPAsyncTCP](https://github.com/me-no-dev/ESPAsyncTCP) (ESP8266)
    * [AsyncTCP](https://github.com/me-no-dev/AsyncTCP) (ESP32)

## Max Client/Slave

The code supports 10 clients and can be changed on the define:

```
#define CLIENTS 10
```

# Connection Pinouts

Connect the pins of the Esp8266 or Esp32 on the Arduino like the table/images below, use a [Logic Level Converter](https://www.sparkfun.com/products/12009) to connect between the ESP and Arduino.

|| ESP8266 | ESP32 | Logic Level Converter | Arduino |
|:-:| :----: | :----: | :-----: | :-----: |
||3.3V | 3.3V | <---> | 5V |
||GND | GND | <---> | GND |
|`SCL`|D1 (GPIO5) | GPIO22 | <---> | A5 |
|`SDA`|D2 (GPIO4) | GPIO21 | <---> | A4 |



<img src="Resources/Fritzing/DuinoCoinI2C/DuinoCoinI2C_LLC.png" alt="DuinoCoinI2C" width="100%">

---

Do you like this project? Please [star this project on GitHub](https://github.com/ricaun/DuinoCoinI2C/stargazers)!