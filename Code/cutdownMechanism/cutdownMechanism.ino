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

// Usage.
// 1. Order PCB and components as provided on the GitHub repository.
// 2. Solder components.
// 3. Upload this code to an Arduino Nano.
// 4. 3D print necessary parts.
// 5. Construct servo release mechanism and place in payload.
// 6. Attach servo to PCB and attach PCB to payload. 
// 7. Power on the PCB on flight day, and press the button to start the fun!

#include <Servo.h>
#include <SoftwareSerial.h>
#include <TinyBME280.h>
#include <TinyGPSPlus.h>
#include <Wire.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

// Settings.
static const uint32_t GPSBaud = 9600;
float refPressure = 101325.00; // Daily pressure at sea level today in Pa.
long cutdownAlt = 10000;       // At this altitude (in meters), drop the glider.
long tooLong = 60;             // How many minutes are too many minutes before dropping the glider?
int restingPoint = 90;         // Resting or middle point of the servo, in degrees.
int openPoint = 180;           // Value in ddgrees where the servo has dropped the glider.
int closedPoint = 0;           // Value in degrees where the servo has not dropped the glider.
#define BUTTON_START           // If enabled, the countdown timer starts when you press the button.
#define DEVMODE                // If enabled, the serial monitor will be enabled and data will be printed on it.

// Pin settings.
static const int TXPin = 3, RXPin = 4;
int servoPin = 5; // Pin that connects to Servo 1 on the PCB.
int button = 9;   // Pin that connects to the button on the PCB.
int LED = 13;     // Built in Arduino Nano LED.

// Variables.
float lat, lon;
long pressure, gpsAlt, bmeAlt, start, time;
int temp, humidity;
bool timerBegun, cutdownState, altValid;

// Declaring objects.
TinyGPSPlus gps;
Servo servo;

// The serial connection to the GPS device.
SoftwareSerial ss(RXPin, TXPin);

tooLong = tooLong * 60000;

void setup() {
  pinMode(button, INPUT);
  pinMode(LED, OUTPUT);

  digitalWrite(LED, HIGH);
  delay(500);
  digitalWrite(LED, LOW);
  delay(500);
  digitalWrite(LED, HIGH);
  delay(500);
  digitalWrite(LED, LOW);

  servo.attach(servoPin);
  delay(100);
  servo.write(closedPoint);
  delay(1000);
  servo.write(restingPoint);
  delay(1000);
  servo.write(openPoint);
  delay(5000);
  servo.write(closedPoint);

#ifdef DEVMODE
  Serial.begin(115200);
#endif
  ss.begin(GPSBaud);
  Wire.begin();
  // BME280setI2Caddress(BME_ADDRESS); // This gave me many issues in testing, don't do it!
  BME280setup();

#ifdef DEVMODE
  Serial.println("StratoSoar cutdown mechanism.");
  Serial.println("Starting in 10 seconds...");
#endif
  delay(10000);
}

void loop() {
#ifdef BUTTON_START
  if (digitalRead(button) && !timerBegun) {
    start = millis();
    digitalWrite(LED, HIGH);
    delay(500);
    digitalWrite(LED, LOW);
    delay(500);
    digitalWrite(LED, HIGH);
    delay(500);
    digitalWrite(LED, LOW);
    timerBegun = 1;
  }
  if (digitalRead(button) && timerBegun) {
    digitalWrite(LED, HIGH);
    delay(5000);
    digitalWrite(LED, LOW);
  }
#endif
#ifndef BUTTON_START
  if (!timerBegun) {
    start = millis();
    timerBegun = 1;
  }
#endif

#ifdef DEVMODE
  if (millis() > 5000 && gps.charsProcessed() < 10) {
    Serial.println(F("No GPS data received: check wiring"));
  }
#endif

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

  pressure = BME280pressure();          // Pressure in Pa.
  temp = BME280temperature();           // Temp in C.
  humidity = BME280humidity();          // Humidity in %RH.
  bmeAlt = BME280altitude(refPressure); // Altitude in meters.
#ifdef DEVMODE
  Serial.print(" BME Alt: ");
  Serial.print(bmeAlt);
  Serial.print(" Pressure: ");
  Serial.print(pressure);
  Serial.print(" Temp: ");
  Serial.print(temp);
  Serial.print(" Humidity: ");
  Serial.println(humidity);
#endif

  time = start - millis();

  if ((altValid && gpsAlt >= cutdownAlt) | (bmeAlt >= cutdownAlt) | (time >= tooLong)) {
    servo.write(openPoint);
    delay(10000);
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    cli(); // Disable interrupts
    sleep_mode(); // Now sleep forever! It's like a dream...
  }
}

// Altitude in meters.
float BME280altitude(float referencePressure) {
  return ((float)-45846.2) * (pow(((float)BME280pressure() / (float)referencePressure), 0.190263) - (float)1);
}