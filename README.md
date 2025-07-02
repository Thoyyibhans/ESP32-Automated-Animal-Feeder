# Smart Farm Feeder (ESP32) 🌿🐄

An open-source, IoT-based Smart Farm Feeder designed to automate animal feeding schedules. Built with an ESP32, this project provides a reliable and efficient way to manage feed distribution using weight sensors, a real-time clock (RTC), and remote control capabilities via Bluetooth. It's perfect for small to medium-sized farms, homesteads, or even automated pet feeding.

This system ensures that animals are fed the right amount at the right time, reducing manual labor and improving feeding accuracy.

---

## ✨ Features

- **⚖️ Precise Weight Measurement**: Uses an HX711 load cell amplifier to accurately measure feed portions.
- **⚙️ Automated Feed Dispensing**: A servo motor controls the gate to dispense feed automatically.
- **🗓️ Scheduled Feeding**: A DS3231 Real-Time Clock (RTC) triggers feeding at pre-set times (e.g., 6:00 AM and 6:00 PM).
- **🔵 Bluetooth Control**: Manually trigger feeding or calibrate the scale using simple commands via Bluetooth Classic serial.
- **🌈 RGB Status Indicator**: A WS2812B LED strip provides visual feedback on the system's status (e.g., idle, feeding, error).
- **🌐 Wi-Fi Ready**: Includes code infrastructure to easily add IoT capabilities like MQTT for remote monitoring or control via platforms like Blynk.
- **🔌 Modular & Open-Source**: Designed with a clean structure, making it easy to modify, expand, and adapt.

---

## 🧰 Hardware & Wiring

### Bill of Materials

All the necessary components to build this project are listed in the [Bill of Materials](hardware/bill-of-materials.md).

### Wiring Diagram

![Wiring Schematic](https://github.com/Thoyyibhans/ESP32-Automated-Animal-Feeder/blob/main/schematic.png)

| Component           | ESP32 Pin        | Connection Details                |
| ------------------- | ---------------- | --------------------------------- |
| **HX711 Load Cell** | `GPIO 26` (DOUT) | Connects to the HX711 DT pin      |
|                     | `GPIO 25` (SCK)  | Connects to the HX711 SCK pin     |
| **Servo Motor SG92R**| `GPIO 18` (PWM)  | Connects to the servo signal wire   |
| **DS3231 RTC** | `GPIO 21` (SDA)  | Connects to the RTC SDA pin       |
|                     | `GPIO 22` (SCL)  | Connects to the RTC SCL pin       |
| **WS2812B LED Strip**|`GPIO 19` (DATA) | Connects to the LED Din pin       |
| **Power** | `5V` / `GND`     | Provide common power and ground   |

---

## 🔧 How It Works

The system operates in a simple, looped sequence:

1.  **Initialization**: On startup, the ESP32 initializes all components: Wi-Fi (optional), Bluetooth, the servo motor, the load cell, RTC, and the LED strip. The scale is tared automatically.
2.  **Idle State**: The system waits for a trigger. The RGB LED glows a steady color (e.g., blue) to indicate it's idle.
3.  **Triggering Feed**:
    * **Scheduled**: The DS3231 RTC is checked continuously. If the current time matches a preset feeding schedule (e.g., 06:00 or 18:00), the feeding cycle begins.
    * **Manual**: A user can connect via a Bluetooth serial terminal and send a command like `feed 0 150` to manually dispense 150 grams of feed.
4.  **Dispensing Feed**:
    * The servo motor opens the feeder gate.
    * The HX711 load cell continuously measures the weight of the dispensed feed.
    * Once the target weight is reached, the servo closes the gate.
    * The RGB LED changes color (e.g., green) during this process.
5.  **Status & Error**: If the load cell fails or another error occurs, the RGB LED will signal an error state (e.g., red).

---

## 🚀 Installation & Setup

### 1. Arduino IDE Setup

1.  **Install the Arduino IDE**: Download and install it from the [official website](https://www.arduino.cc/en/software).
2.  **Add ESP32 Board Manager**:
    * Go to `File > Preferences`.
    * In "Additional Board Manager URLs," add: `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`.
    * Go to `Tools > Board > Boards Manager`, search for "esp32," and install it.
3.  **Install Libraries**:
    * Open the Arduino IDE and go to `Sketch > Include Library > Manage Libraries`.
    * Install the following libraries:
        * `HX711-ADC` by Olav Kallhovd
        * `RTClib` by Adafruit
        * `Adafruit NeoPixel` by Adafruit

### 2. Upload the Code

1.  Clone this repository or download the code.
2.  Open the `Arduino/smart_feeder/smart_feeder.ino` file in the Arduino IDE.
3.  Connect your ESP32 to your computer via USB.
4.  Select your ESP32 board (`ESP32 Dev Module`) and the correct COM port under the `Tools` menu.
5.  Click the **Upload** button.

### 3. Calibration

The load cell must be calibrated for accurate measurements.

1.  Connect to the ESP32 via the Serial Monitor or a Bluetooth terminal.
2.  Send the command `calibrate`. The system will guide you through placing a known weight on the scale and will save the calibration factor to memory.

---

## 🔮 Optional Expansions

This project is a great foundation for more advanced features:

-   **☀️ Solar Power**: Add a solar panel and battery management system (like a TP4056 with protection) to make the feeder self-sufficient.
-   ** MQTT/Blynk Integration**: Use the built-in Wi-Fi capability to send data to an IoT dashboard for remote monitoring of feed levels and feeding logs.
-   **📷 Camera Module**: Integrate an ESP32-CAM to visually monitor the animals and feeding area.
-   **🆔 Animal Identification**: Use an RFID/NFC reader (like an MFRC522) to identify specific animals and provide custom feed portions.

---

## ✍️ Author

Created by **[Thoyyibhans]**.

Connect with me:

[![GitHub Badge](https://img.shields.io/badge/GitHub-100000?style=for-the-badge&logo=github&logoColor=white)](https://github.com/Thoyyibhans)
[![LinkedIn Badge](https://img.shields.io/badge/LinkedIn-0077B5?style=for-the-badge&logo=linkedin&logoColor=white)](https://www.linkedin.com/in/thoyyibhans/)

---

## 📄 License

This project is licensed under the **MIT License**. See the `LICENSE` file for more details.
