/*
  Smart Farm Feeder - ESP32
  An open-source automated feeder using an ESP32, HX711 load cell, servo,
  DS3231 RTC, and Bluetooth for control.

  Features:
  - Weight-based feeding
  - Scheduled feeding via RTC
  - Manual control via Bluetooth
  - WS2812B status indicator
  - Modular code structure

  Required Libraries:
  - HX711-ADC: https://github.com/olkal/HX711_ADC
  - RTClib: https://github.com/adafruit/RTClib
  - Adafruit NeoPixel: https://github.com/adafruit/Adafruit_NeoPixel
*/

// =============================================================================
// LIBRARIES
// =============================================================================
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_NeoPixel.h>
#include <ESP32Servo.h>
#include "RTClib.h"
#include "BluetoothSerial.h"
#include "HX711_ADC.h"

// =============================================================================
// CONFIGURATION & PIN DEFINITIONS
// =============================================================================

// -- Hardware Pins
const int HX711_DOUT_PIN = 26;
const int HX711_SCK_PIN = 25;
const int SERVO_PIN = 18;
const int NEOPIXEL_PIN = 19;

// -- System Parameters
const int NEOPIXEL_COUNT = 4; // Number of LEDs on the strip
const int SERVO_OPEN_ANGLE = 90;
const int SERVO_CLOSE_ANGLE = 0;
const float DEFAULT_CALIBRATION_FACTOR = 2280.0; // Adjust this via calibration
const long FEEDING_TIMEOUT_MS = 30000; // 30 seconds timeout for dispensing

// -- Feeding Schedule (24-hour format)
const int SCHEDULED_HOUR_1 = 6;
const int SCHEDULED_MINUTE_1 = 0;
const int SCHEDULED_HOUR_2 = 18;
const int SCHEDULED_MINUTE_2 = 0;
const float SCHEDULED_WEIGHT_G = 250.0; // Grams to dispense on schedule

// -- Wi-Fi Credentials (Optional, for future use)
const char* WIFI_SSID = "Your_WiFi_SSID";
const char* WIFI_PASSWORD = "Your_WiFi_Password";

// =============================================================================
// GLOBAL OBJECTS
// =============================================================================
Adafruit_NeoPixel pixels(NEOPIXEL_COUNT, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);
Servo feedServo;
RTC_DS3231 rtc;
BluetoothSerial SerialBT;
HX711_ADC LoadCell(HX711_DOUT_PIN, HX711_SCK_PIN);

// =============================================================================
// SYSTEM STATE & STATUS
// =============================================================================
enum SystemStatus { IDLE, FEEDING, ERROR, CALIBRATING };
SystemStatus currentStatus = IDLE;

// =============================================================================
// LED STATUS FUNCTIONS
// =============================================================================
void setStatusLED(SystemStatus status) {
  currentStatus = status;
  switch (status) {
    case IDLE:
      pixels.fill(pixels.Color(0, 0, 255)); // Blue for Idle
      break;
    case FEEDING:
      pixels.fill(pixels.Color(0, 255, 0)); // Green for Feeding
      break;
    case ERROR:
      pixels.fill(pixels.Color(255, 0, 0)); // Red for Error
      break;
    case CALIBRATING:
      pixels.fill(pixels.Color(255, 255, 0)); // Yellow for Calibrating
      break;
  }
  pixels.show();
}

// =============================================================================
// WIFI & BLUETOOTH FUNCTIONS
// =============================================================================
void setupWiFi() {
  // Optional: For future MQTT/Blynk integration
  // WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  // Serial.print("Connecting to WiFi...");
  // while (WiFi.status() != WL_CONNECTED) {
  //   delay(500);
  //   Serial.print(".");
  // }
  // Serial.println(" Connected!");
}

void setupBluetooth() {
  if (!SerialBT.begin("SmartFarmFeeder")) {
    Serial.println("An error occurred initializing Bluetooth");
    setStatusLED(ERROR);
  } else {
    Serial.println("Bluetooth device available");
  }
}

// =============================================================================
// CORE FEEDING & SCALE LOGIC
// =============================================================================
void setupScale() {
  LoadCell.begin();
  float calibrationValue = DEFAULT_CALIBRATION_FACTOR; // Future: read from EEPROM
  LoadCell.set_scale(calibrationValue);
  LoadCell.tare(); // Zero the scale
  Serial.println("Load cell tared. Ready to weigh.");
}

bool dispenseFeed(float targetWeight) {
  Serial.printf("Dispensing %.1f grams of feed...\n", targetWeight);
  setStatusLED(FEEDING);
  
  feedServo.write(SERVO_OPEN_ANGLE);
  
  long startTime = millis();
  while (true) {
    if (millis() - startTime > FEEDING_TIMEOUT_MS) {
      Serial.println("Error: Feeding timed out.");
      feedServo.write(SERVO_CLOSE_ANGLE);
      setStatusLED(ERROR);
      return false;
    }

    if (LoadCell.is_ready()) {
      float currentWeight = LoadCell.get_units(10);
      Serial.printf("Current weight: %.1f g\n", currentWeight);
      
      if (currentWeight >= targetWeight) {
        break; // Target weight reached
      }
    }
    delay(100);
  }
  
  feedServo.write(SERVO_CLOSE_ANGLE);
  Serial.println("Feeding complete.");
  setStatusLED(IDLE);
  
  // Tare the scale again to be ready for the next reading
  LoadCell.tare();
  delay(1000);

  return true;
}

void calibrateScale() {
  setStatusLED(CALIBRATING);
  Serial.println("\n>>> Scale Calibration <<<");
  SerialBT.println("\n>>> Scale Calibration <<<");

  Serial.println("Step 1: Remove all weight from the scale.");
  SerialBT.println("Step 1: Remove all weight from the scale.");
  Serial.println("Send 'ok' when ready.");
  SerialBT.println("Send 'ok' when ready.");
  
  while (!SerialBT.available() && !Serial.available()) { delay(100); }
  while (SerialBT.available()) SerialBT.read();
  while (Serial.available()) Serial.read();

  LoadCell.tare();
  Serial.println("Scale tared.");
  SerialBT.println("Scale tared.");

  Serial.println("\nStep 2: Place a known weight on the scale.");
  SerialBT.println("\nStep 2: Place a known weight on the scale.");
  Serial.println("Then, send the weight in grams (e.g., '1000').");
  SerialBT.println("Then, send the weight in grams (e.g., '1000').");

  float knownWeight = 0;
  while (knownWeight == 0) {
    if (SerialBT.available()) {
      knownWeight = SerialBT.parseFloat();
    }
    if (Serial.available()) {
      knownWeight = Serial.parseFloat();
    }
    delay(100);
  }
  
  while (SerialBT.available()) SerialBT.read();
  while (Serial.available()) Serial.read();

  float newCalibrationValue = LoadCell.get_value(10) / knownWeight;
  LoadCell.set_scale(newCalibrationValue);

  Serial.printf("Calibration complete! New factor: %.2f\n", newCalibrationValue);
  SerialBT.printf("Calibration complete! New factor: %.2f\n", newCalibrationValue);
  Serial.println("This value is NOT saved permanently yet. Update DEFAULT_CALIBRATION_FACTOR in the code.");
  SerialBT.println("This value is NOT saved permanently yet. Update it in the code.");

  setStatusLED(IDLE);
}


// =============================================================================
// BLUETOOTH COMMAND HANDLER
// =============================================================================
void handleBluetoothCommands() {
  if (SerialBT.available()) {
    String cmd = SerialBT.readStringUntil('\n');
    cmd.trim();
    Serial.printf("Received Bluetooth command: %s\n", cmd.c_str());

    if (cmd.startsWith("feed")) {
      float weight = cmd.substring(cmd.indexOf(" ") + 1).toFloat();
      if (weight > 0) {
        dispenseFeed(weight);
      } else {
        SerialBT.println("Invalid weight. Usage: feed <grams>");
      }
    } else if (cmd.equals("status")) {
      float currentWeight = LoadCell.get_units(5);
      DateTime now = rtc.now();
      SerialBT.printf("Status: %d | Weight: %.1fg | Time: %02d:%02d\n", currentStatus, currentWeight, now.hour(), now.minute());
    } else if (cmd.equals("calibrate")) {
        calibrateScale();
    } else {
      SerialBT.println("Unknown command. Try: feed <grams>, status, calibrate");
    }
  }
}

// =============================================================================
// RTC & SCHEDULING
// =============================================================================
void checkSchedule() {
  static bool fedAtSchedule1 = false;
  static bool fedAtSchedule2 = false;

  DateTime now = rtc.now();

  // Check first schedule
  if (now.hour() == SCHEDULED_HOUR_1 && now.minute() == SCHEDULED_MINUTE_1) {
    if (!fedAtSchedule1) {
      Serial.println("Scheduled feeding time 1 triggered.");
      if (dispenseFeed(SCHEDULED_WEIGHT_G)) {
        fedAtSchedule1 = true;
      }
    }
  } else {
    fedAtSchedule1 = false; // Reset flag after the scheduled minute passes
  }

  // Check second schedule
  if (now.hour() == SCHEDULED_HOUR_2 && now.minute() == SCHEDULED_MINUTE_2) {
    if (!fedAtSchedule2) {
      Serial.println("Scheduled feeding time 2 triggered.");
      if (dispenseFeed(SCHEDULED_WEIGHT_G)) {
        fedAtSchedule2 = true;
      }
    }
  } else {
    fedAtSchedule2 = false; // Reset flag
  }
}

void setupRTC() {
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    setStatusLED(ERROR);
    while (1);
  }
  if (rtc.lostPower()) {
    Serial.println("RTC lost power, let's set the time!");
    // The following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
}

// =============================================================================
// SETUP & LOOP
// =============================================================================
void setup() {
  Serial.begin(115200);
  while (!Serial);
  Serial.println("\nInitializing Smart Farm Feeder...");

  // Initialize LEDs
  pixels.begin();
  pixels.setBrightness(50);
  setStatusLED(IDLE);

  // Initialize Servo
  ESP32PWM::allocateTimer(0);
  feedServo.attach(SERVO_PIN);
  feedServo.write(SERVO_CLOSE_ANGLE); // Ensure gate is closed on startup

  // Initialize RTC
  setupRTC();

  // Initialize Load Cell
  setupScale();

  // Initialize Bluetooth
  setupBluetooth();
  
  // Optional Wi-Fi
  // setupWiFi();

  Serial.println("System setup complete. Waiting for commands or schedule.");
}

void loop() {
  // Only check schedule and commands if not in an error state
  if (currentStatus != ERROR) {
    checkSchedule();
    handleBluetoothCommands();
  }
  
  // Small delay to prevent busy-waiting
  delay(100);
}
