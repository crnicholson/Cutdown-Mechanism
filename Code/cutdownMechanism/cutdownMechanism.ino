/*
cutdownMechanism.ino, part of StratoSoar MK2, for releasing a glider from a high altitude balloon payload.
Copyright (C) 2024 Charles Nicholson

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

// https://github.com/crnicholson/StratoSoar-MK2/.

// Usage:
// 1. Order PCB and components as provided on the GitHub repository.
// 2. Solder components.
// 3. Upload this code to an Arduino Nano.
// 4. 3D print necessary parts.
// 5. Construct servo release mechanism and place in payload.
// 6. Attach servo to PCB and attach PCB to payload.
// 7. Power on the PCB on flight day, and press the button to start the fun!

#include <LoRa.h>
#include <SPI.h>
#include <Servo.h>
#include <SoftwareSerial.h>
#include <TinyBME280.h>
#include <TinyGPSPlus.h>
#include <Wire.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#include "headers/settings.h"

// Variables.
float lat, lon, pressure;
long gpsAlt, bmeAlt, start, time, last;
int temp, humidity;
bool timerBegun, cutdownState, altValid;

Servo servo;

#ifdef GPS
TinyGPSPlus gps;
SoftwareSerial ss(RX_PIN, TX_PIN);
#endif

long tooLongMS = TOO_LONG * 60000;

void setup() {
  pinMode(BUTTON, INPUT);
  pinMode(LED, OUTPUT);

  blink();

#ifndef REMOTE
  servo.attach(SERVO_PIN);
  delay(100);
  servo.write(CLOSED_POINT);
  delay(1000);
  servo.write(RESTING_POINT);
  delay(1000);
  servo.write(OPEN_POINT);
  delay(5000);
  servo.write(CLOSED_POINT);
#endif

#ifdef DEVMODE
  Serial.begin(MONITOR_BAUD);
#endif
#ifdef GPS
  ss.begin(GPS_BAUD);
#endif
  Wire.begin();

#ifndef REMOTE
  // BME280setI2Caddress(BME_ADDRESS); // This gave me many issues in testing, don't do it!
  BME280setup();
#endif

#ifdef LORA_MODE
  LoRa.setPins(SS_PIN, RESET_PIN, DIO0_PIN); // Has to be before LoRa.begin().

  if (!LoRa.begin(FREQUENCY)) {
#ifdef DEVMODE
    Serial.println("Starting LoRa failed!");
#endif
    while (1)
      ;
  } else {
    Serial.println("LoRa started.");
  }

  LoRa.setSyncWord(SYNC_WORD);               // Defined in settings.h.
  LoRa.setSpreadingFactor(SPREADING_FACTOR); // Defined in settings.h.
  LoRa.setSignalBandwidth(BANDWIDTH);        // Defined in settings.h.
  LoRa.crc();                                // Use a checksum.
#endif

#ifdef DEVMODE
#ifdef LORA_MODE
  Serial.print("StratoSoar LoRa cutdown mechanism ");
#ifdef REMOTE
  Serial.print("remote. Press button to trigger release.");
#endif
#ifndef REMOTE
  Serial.println("receiver. Waiting for button press on remote to release.");
#endif
#endif
#ifndef LORA_MODE
  Serial.println("StratoSoar normal cutdown mechanism.");
#endif
  Serial.println("Starting in 1 second...");
#endif
  delay(1000);
}

void loop() {
#ifndef LORA_MODE
#ifdef BUTTON_START
  if (digitalRead(BUTTON) && !timerBegun) {
    start = millis();
    blink();
    timerBegun = 1;
  }
  if (digitalRead(BUTTON) && timerBegun) {
    digitalWrite(LED, HIGH);
    delay(2000);
    digitalWrite(LED, LOW);
  }
#endif

#ifndef BUTTON_START
  if (!timerBegun) {
    start = millis();
    timerBegun = 1;
  }
#endif

#ifdef BUTTON_START
  if (!timerBegun) {
#ifdef DEVMODE
    Serial.println("Press the button to start the program!");
#endif
    pulse();
  }
#endif

#ifdef GPS
  if (timerBegun) {
    if (millis() > 5000 && gps.charsProcessed() < 10) {
#ifdef DEVMODE
      Serial.println(F("No GPS data received: check wiring."));
#endif
      longPulse();
    }
  }

  altValid = gps.altitude.isValid();

  if (altValid && gps.location.isValid()) {
    lat = gps.location.lat();
    lon = gps.location.lng();
    gpsAlt = gps.altitude.meters();

#ifdef DEVMODE
    Serial.print("Lat: ");
    Serial.print(lat, 6);
    Serial.print(" Lon: ");
    Serial.print(lon, 6);
    Serial.print(" GPS Alt: ");
    Serial.print(gpsAlt);
#endif
  }
#endif

  pressure = BME280pressure();           // Pressure in Pa.
  temp = BME280temperature();            // Temp in C * 100.
  humidity = BME280humidity();           // Humidity in %RH * 100.
  bmeAlt = BME280altitude(REF_PRESSURE); // Altitude in meters.

#ifdef DEVMODE
  Serial.print(" BME Alt (m): ");
  Serial.print(bmeAlt);
  Serial.print(" Pressure (hPa): ");
  Serial.print(pressure / 100);
  Serial.print(" Temp (C): ");
  Serial.print(temp / 100);
  Serial.print(" Humidity (%RH): ");
  Serial.println(humidity / 100);
#endif

  time = millis() - start;

#ifdef GPS
  // Open the servo and go into an infinite sleep.
  if ((altValid && gpsAlt >= CUTDOWN_ALT) | (bmeAlt >= CUTDOWN_ALT) | (time >= tooLongMS)) {
    servo.write(OPEN_POINT);
    delay(20000);
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    cli();        // Disable interrupts.
    sleep_mode(); // Now sleep forever! It's like a dream...
  }
#endif
#ifndef GPS
  // Open the servo and go into an infinite sleep.
  if ((gpsAlt >= CUTDOWN_ALT) | (bmeAlt >= CUTDOWN_ALT) | (time >= tooLongMS)) {
#ifdef DEVMODE
    Serial.println("Opening servo and sleeping for ever.");
#endif
    servo.write(OPEN_POINT);
    delay(20000);
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    cli();        // Disable interrupts.
    sleep_mode(); // Now sleep forever! It's like a dream...
  }
  gpsAlt += TESTING_INCREMENTS;
#endif
  delay(250);
#endif
#ifdef LORA_MODE
#ifdef REMOTE
#ifdef DEVMODE
  if (millis() - last > 1000) {
    Serial.println("Waiting for button press to trigger release.");
    last = millis();
  }
#endif
  if (digitalRead(BUTTON)) {
    Serial.println("Button pressed, sending release signal.");
    LoRa.beginPacket();
    LoRa.print(1);
    LoRa.endPacket();
    delay(5000);
  }
#endif
#ifndef REMOTE
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    while (LoRa.available()) {
      if (LoRa.read()) {
        servo.write(OPEN_POINT);
        blink();
        delay(20000);
        servo.write(CLOSED_POINT);
      }
    }
  }
#endif
#endif
}

// Altitude in meters.
float BME280altitude(float referencePressure) {
  return ((float)-45846.2) * (pow(((float)BME280pressure() / (float)referencePressure), 0.190263) - (float)1);
}

// Blink LED.
void blink() {
  digitalWrite(LED, HIGH);
  delay(500);
  digitalWrite(LED, LOW);
  delay(500);
  digitalWrite(LED, HIGH);
  delay(500);
  digitalWrite(LED, LOW);
}

// Pulse LED.
void pulse() {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  delay(1000);
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
}

// Long pulse the LED.
void longPulse() {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  delay(1000);
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  delay(1000);
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  delay(1000);
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
}