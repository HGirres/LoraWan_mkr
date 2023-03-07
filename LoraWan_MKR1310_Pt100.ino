

/*
  Lora Send And Receive
  This sketch demonstrates how to send and receive data with the MKR WAN 1300/1310 LoRa module.
  This example code is in the public domain.
*/


#include <ArduinoLowPower.h>
#include <MKRWAN.h>
#include <Adafruit_MAX31865.h>

 #ifdef ADAFRUIT
 #define RREF     430.0 // Version Adafruit : résistance de réf de 430 ohms
 #else
 #define RREF     400.0 // Version Analog device : résistance de réf de 400ohms
 #endif
 #define RNOMINAL  100.0 // Utilisation d'une Pt100
 #define ALIM_MAX31865 0 // Utilisation de la pin "0" pour alimenter le module MAX31865
 #define CS_MAX31865 11 // Pin du CS du protocole SPI vers le MAX31865


 LoRaModem modem(Serial1);
 
 Adafruit_MAX31865 thermo = Adafruit_MAX31865(CS_MAX31865); // Version SPI "HARD" sur mkr WAN 1310
 
//#include "arduino_secrets.h"
// Please enter your sensitive data in the Secret tab or arduino_secrets.h
String appEui = "000D000A00000001";
String appKey = "0123456789ABCDEF0123456789ABCDEF";

void setup() {
  // put your setup code here, to run once:
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  pinMode(ALIM_MAX31865,OUTPUT);
  digitalWrite(ALIM_MAX31865,LOW);
  thermo.begin(MAX31865_4WIRE); // Configuration du MAX31865 en mesure 4 fils
  Serial.begin(115200);
  while (!Serial);
  if (!modem.begin(EU868)) {
    Serial.println("Failed to start module");
    while (1) {}
  };
  Serial.print("Your module version is: ");
  Serial.println(modem.version());
  Serial.print("Your device EUI is: ");
  Serial.println(modem.deviceEUI());

  int connected = modem.joinOTAA(appEui, appKey);
  if (!connected) {
    Serial.println("Something went wrong; are you indoor? Move near a window and retry");
    while (1) {}
  }

  // Set poll interval to 60 secs.
  modem.minPollInterval(60);
  // NOTE: independently by this setting the modem will
  // not allow to send more than one message every 2 minutes,
  // this is enforced by firmware and can not be changed.
  delay(5000); // Temporisation au démarrage de l'appli, pour avoir le temps de profiter de l'état éveillé du MKR
}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH);
  digitalWrite(ALIM_MAX31865,HIGH);
  uint16_t rtd = thermo.readRTD(); // Mesure de la température
  int16_t data = thermo.temperature(RNOMINAL, RREF); // Extration de la température
  data = (data + 273.15) * 10 ;
  // Check and print any faults
  uint8_t fault = thermo.readFault();
  if (fault) {
    Serial.print("Fault 0x"); Serial.println(fault, HEX);
    if (fault & MAX31865_FAULT_HIGHTHRESH) {
      Serial.println("RTD High Threshold"); 
    }
    if (fault & MAX31865_FAULT_LOWTHRESH) {
      Serial.println("RTD Low Threshold"); 
    }
    if (fault & MAX31865_FAULT_REFINLOW) {
      Serial.println("REFIN- > 0.85 x Bias"); 
    }
    if (fault & MAX31865_FAULT_REFINHIGH) {
      Serial.println("REFIN- < 0.85 x Bias - FORCE- open"); 
    }
    if (fault & MAX31865_FAULT_RTDINLOW) {
      Serial.println("RTDIN- < 0.85 x Bias - FORCE- open"); 
    }
    if (fault & MAX31865_FAULT_OVUV) {
      Serial.println("Under/Over voltage"); 
    }
    thermo.clearFault();
  }
  digitalWrite(ALIM_MAX31865,LOW);
  int err;
  modem.beginPacket();
  modem.write(data);
  err = modem.endPacket(true);
  if (err > 0) {
    Serial.println("Message sent correctly!");
  } else {
    Serial.println("Error sending message :(");
    Serial.println("(you may send a limited amount of messages per minute, depending on the signal strength");
    Serial.println("it may vary from 1 message every couple of seconds to 1 message every minute)");
  }
/*  

  if (!modem.available()) {
    Serial.println("No downlink message received at this time.");
    return;
  }
  char rcv[64];
  int i = 0;
  while (modem.available()) {
    rcv[i++] = (char)modem.read();
  }
  Serial.print("Received: ");
  for (unsigned int j = 0; j < i; j++) {
    Serial.print(rcv[j] >> 4, HEX);
    Serial.print(rcv[j] & 0xF, HEX);
    Serial.print(" ");
  }
  Serial.println();
  */
  digitalWrite(LED_BUILTIN, LOW);
    LowPower.sleep(60000);
  //delay(60000);
}
