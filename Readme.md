# Smart Irrigation System with Rainwater Harvesting

## Description
This project is a smart irrigation system prototype designed to optimize water usage through rainwater harvesting and IoT-based automation. Built with an ESP32 microcontroller, the system continuously monitors soil moisture and water tank levels using capacitive sensors and ultrasonic sensors. It features a dual-tank architecture that prioritizes collected rainwater for plant irrigation. When soil moisture drops below a specific threshold and sufficient water is available, the ESP32 automatically activates miniature water pumps to hydrate the plants. 

Additionally, the system integrates a Decision Tree machine learning model. By analyzing real-time temperature and humidity from a DHT11 sensor, it provides localized weather predictions. Users can monitor sensor data, receive emergency tank alerts, and manually control the pumps remotely via the Blynk IoT dashboard and a Telegram Bot integration. This ensures an efficient and sustainable approach to small-scale agriculture and home gardening.

## Features
* **Automated Irrigation:** Pumps activate automatically when soil moisture falls below 30%.
* **Rainwater Harvesting:** Utilizes a primary rainwater tank and a clean water tank for sustainable water management.
* **AI Weather Prediction:** Features an onboard Decision Tree model to forecast weather conditions based on temperature and humidity.
* **Remote Monitoring:** View real-time sensor data and control pumps manually using the Blynk IoT app.
* **Instant Alerts:** Receive push notifications via Telegram for empty tanks, full tanks, and leak detection.

## Hardware Requirements
* ESP32 DevKit V1
* DHT11 Temperature and Humidity Sensor
* 3x Capacitive Soil Moisture Sensors
* 2x HC-SR04 Ultrasonic Sensors
* 4-Channel 5V Relay Module
* 3x 5V Mini Water Pumps

## Software Requirements
* Blynk IoT Account and App
* Telegram Bot API Token
* Required Libraries: `WiFi.h`, `BlynkSimpleEsp32.h`, `DHT.h`, `HTTPClient.h`, `WiFiClientSecure.h`

## Setup Instructions
1. Clone this repository to your local machine.
2. Open the main `.ino` file.
3. Update the Wi-Fi credentials, Blynk Auth Token, and Telegram Bot details in the code.
4. Ensure the `weather.h` file is located in the same directory as your main sketch.
5. Flash the code to your ESP32 board.