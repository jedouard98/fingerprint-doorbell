# Fingerprint Doorbell with ESP8266

Ever wanted the ability to know who was at the door with just the touch of a finger? This developing project is one that combines the functionality of the ESP8266 ESP-12E NODEMCU with the KOOKYE Fingerprint Reader Sensor Module to do just that. A person using the device will place their finger onto the scanner, which then triggers the esp8266 to send the owner a notifcation of who has used this device, and subsequently, who is at the door!

## Getting Started: Hardware

### Materials

Building this project only requires the use of a breadboard, wires, and the modules listed below.

* ESP8266 ESP-12E LUA NODEMCU
* KOOKYE Fingerprint Reader Sensor Module

### Schematic


## Getting Started: Software

## Installation

This project was created using [Arduino v1.8.2](https://www.arduino.cc/en/Main/OldSoftwareReleases/).

### Libraries

Building this project requires the inclusion of several libraries, most of which can be downloaded from the Arduino Library Manager. They're listed as follows:

* ESP8266Wifi
* EEPROM
* Software Serial
* ArduinoJSON
* peng_fei_Fingerprint*
* [Firebase Arduino**](https://github.com/firebase/firebase-arduino)

\*This was software that came with the fingerprint sensor and was used in this project. Additionally, one change was made to a file in this library to work with the ESP8266. 

This snippet of code
```
#include <util/delay.h>
```
was changed to this

```
#ifdef __AVR
   #include <avr/pgmspace.h>
#elif defined(ESP8266)
  #include <pgmspace.h>
#endif
```

\**This can be downloaded via the Arduino Library Manager, but only the older version of it. To install this, clone [the repo](https://github.com/firebase/firebase-arduino) and place it in the same directory (or change the # include statement with the appropriate path).

### API Keys

API keys were retrieved for use in this project. This includes ones for a Firebase database (with read and write permissions turned on) and pushingbox deviceIDs for a pushbullet notification. Add these to the code for the project to function properly.

## Deployment

After downloading the appropriate libraries and retrieving the necessary parts, use the Arduino flash tool to get the software onto the ESP8266. These settings in Tools were set as follows:
```
Board: NodeMCU 1.0 (ESP-12E Module)
CPU Frequency: 80 MHz
Flash Size: 4M (3M SPIFFS)
Upload Speed: 115200
Port: /dev/cu.SLAB_USBtoUART
```

## Version

This is version 1.2 of this project. Future updates will hope to improve the fingerprint identifying ability by making the enroll and identify sketches into a mode set by a button. Also hope to add an additional reset button that allows for the user's firebase database to be cleared as well as the fingerprint's internal data.


## Acknowledgements

Credit goes to Vaduva Ionut Lucian, owner of GeeksTips.com, for the helpful tutorial on interfacing the ESP8266 with the pushingbox api. Additional credit goes to the creators of the Firebase Arduino library.