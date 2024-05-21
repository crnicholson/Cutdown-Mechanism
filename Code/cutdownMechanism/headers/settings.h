/*
settings.h, part of StratoSoar MK2, for releasing a glider from a high altitude balloon payload.
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

// General settings.
#define GPS_BAUD 9600          // Baud rate of the serial communications to the GPS.
#define MONITOR_BAUD 9600      // Baud rate of the serial monitor for showing data.
#define REF_PRESSURE 101325.00 // Daily pressure at sea level today in Pa.
#define CUTDOWN_ALT 10000      // At this altitude (in meters), drop the glider.
#define TOO_LONG 1            // How many minutes are too many minutes before dropping the glider?
#define RESTING_POINT 90       // Resting or middle point of the servo, in degrees.
#define OPEN_POINT 180         // Value in degrees where the servo has dropped the glider.
#define CLOSED_POINT 0         // Value in degrees where the servo has not dropped the glider.
#define TESTING_ALT 0          // Altitude in meters to test the cutdown mechanism.
#define TESTING_INCREMENTS 0   // Number that is added to the altitude every pass through the loop, if not using the GPS module.

// Enables.
// #define GPS          // Do you need the GPS module?
#define BUTTON_START // If enabled, the countdown timer starts when you press the button. Otherwise, it starts shortly after power-on.
#define DEVMODE // If enabled, the serial monitor will be enabled and data will be printed on it.
// #define LORA_MODE // If enabled, LoRa can be used to activate the cutdown mechanism. Note that this should only be used for ground testing.
#define REMOTE // If enabled, the board acts like a remote to activate the cutdown mechanism. If disabled, the board acts like the receiver for the cutdown mechanism.

// LoRa settings.
#define SYNC_WORD 0xFB      // Only other devices with this sync word can receive your broadcast.
#define FREQUENCY 433E6     // Frequency of your LoRa module.
#define SPREADING_FACTOR 10 // Choose this based on this https://forum.arduino.cc/t/what-are-the-best-settings-for-a-lora-radio/449528.
#define BANDWIDTH 62.5E3    // Choose this based on this https://forum.arduino.cc/t/what-are-the-best-settings-for-a-lora-radio/449528.

// Pin settings.
#define TX_PIN 3 // For the SoftwareSerial connection to the GPS module.
#define RX_PIN 4 // For the SoftwareSerial connection to the GPS module.
#define SERVO_PIN 5
#define BUTTON 2
#define LED 13      // Built-in Arduino Nano LED.
#define DIO0_PIN 2  // For the LoRa module.
#define RESET_PIN 4 // For the LoRa module.
#define SS_PIN 10   // For the SPI of the LoRa module.