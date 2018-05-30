#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
#include <EEPROM.h>
#include "FirebaseArduino.h"
#include <peng_fei_Fingerprint.h>

// WIFI constants
#define WIFI_SSID ""
#define WIFI_PASSWORD ""

// Firebase constants
#define FIREBASE_HOST ""
#define FIREBASE_AUTH ""

// ESP8266 pin mapping
#define D6 12
#define D5 14

// Pushing box constants for notification
String deviceId = "";
const char* logServer = "api.pushingbox.com";

// Address for EEPROM reading and writing
int addr = 0;
byte id;

// pin D6 is IN from sensor (GREEN wire)
// pin D5 is OUT from arduino (WHITE wire)
SoftwareSerial mySerial(D6, D5);

// Setup for finger print sensor!
peng_fei_Fingerprint finger = peng_fei_Fingerprint(&mySerial);

void setup()  
{
  Serial.begin(9600);

  // necessary for esp8266 eeprom reading and writing
  EEPROM.begin(4);

  //if first time flashing code, write 0 to this address. Otherwise, comment this out.
  EEPROM.write(addr, 0);

  // set the data rate for the sensor serial port
  finger.begin(57600);

  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor !!!!");
    while (1);
  }

  // connect to wifi.
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("connected: ");
  Serial.println(WiFi.localIP());

    
  // start database
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
}

void loop()                     // run over and over again
{
  // continously look for a finger to inspect
  while (!  getFingerprintEnroll() );
}

uint8_t getFingerprintEnroll() {
  Serial.print("Enrolling ID #");
  id = EEPROM.read(addr);
  Serial.println(id);

  //first enroll the finger as though it were a new fingerprint
  
  uint8_t p = -1;
  Serial.println("Waiting for valid finger to enroll");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error0");
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error1");
      return p;
  }
  
  p = -1;
  Serial.println("Place same finger again");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.print(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error3");
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error4");
      return p;
  }

  // OK converted!
  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    Serial.println("Prints matched!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    Serial.println("Fingerprints did not match");
    return p;
  } else {
    Serial.println("Unknown error5");
    return p;
  }  

  // After creating a fingerprint, look for a matching fingerprint in the database. If not found
  // then add it to the sensor's database. If found, do not store the created fingerprint

  p = finger.fingerFastSearch();
  if (p == FINGERPRINT_OK) {
    Serial.println("Found a print match!");
    Serial.print("ID of print match is: ");
    Serial.println(finger.fingerID);
    sendNotification(finger.fingerID);
    delay(2000);
    return p;
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println("Did not find a match");
    Serial.println("Storing the model as a result...");
    
    p = finger.storeModel(id);
    if (p == FINGERPRINT_OK) {
    Serial.print("Stored! #");
    Serial.println(id);
    
    // set firebase to have the id number of fingerprint!
    Firebase.setString(String(id), "An Unknown Person");
     // handle error
    if (Firebase.failed()) {
        Serial.print("setting /number failed:");
        Serial.println(Firebase.error());  
        return p;
    }
    sendNotification(id);
    id = id + 1;
    EEPROM.write(addr, id);
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not store in that location");
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  } 
  
    delay(2000);
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }   
}

void sendNotification(int id) {
  // get name associated with id
  String name = Firebase.getString(String(id));
  WiFiClient client;

  //connect to the server
  if (client.connect(logServer, 80)) {
    Serial.println("- succesfully connected");
    String postStr = "devid=";
    postStr += String(deviceId);
    postStr += "&name_parameter=";
    postStr += name;
    //Serial.println("- sending data...");
    client.print("POST /pushingbox HTTP/1.1\n");
    client.print("Host: api.pushingbox.com\n");
    client.print("Connection: close\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(postStr.length());
    client.print("\n\n");
    client.print(postStr);
  }
  client.stop();
}

