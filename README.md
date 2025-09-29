🌱 ESP32 Smart Soil Moisture Control System with Blynk Cloud

This project is an automatic soil moisture control system powered by ESP32, featuring a Soil Moisture Sensor, DHT11/DHT22, and a Relay Module for Water Pump Control.
The system monitors temperature, humidity, and soil moisture, while controlling the water pump automatically or manually via Blynk IoT Cloud.

⚡ Features

📊 Real-time monitoring of:

Temperature (°C)

Air Humidity (%)

Soil Moisture Level

💧 Automatic water pump control based on conditions:

Soil is too dry

Low air humidity

High temperature

📱 Manual control of the water pump via Blynk App or Web Dashboard.

🌐 Seamless IoT integration with Blynk Cloud for remote monitoring and control.

💡 Built-in LED indicator:

Blinking → System idle

Solid ON → Pump active

🛠️ Hardware Requirements

ESP32 Development Board

DHT11 / DHT22 (Temperature & Humidity Sensor)

Soil Moisture Sensor (Analog)

5V Relay Module

Mini Water Pump

Breadboard, Jumper Wires, and Power Supply

📦 Software Requirements

Arduino IDE

ESP32 Board Manager

Blynk Library

Adafruit DHT Sensor Library

⚙️ Pin Configuration
Component	ESP32 Pin
DHT11/DHT22	GPIO 23
Soil Moisture	GPIO 4
Relay Module	GPIO 25
Onboard LED	GPIO 2
📲 Blynk Virtual Pin Mapping
Virtual Pin	Function
V0	Temperature (°C)
V1	Air Humidity (%)
V2	Soil Moisture (ADC)
V3	Pump Status (ON/OFF)
V4	Manual Pump Control
🚀 How It Works

Sensors collect data (temperature, humidity, and soil moisture).

ESP32 processes the readings and decides whether to activate the pump.

Data is sent to Blynk Cloud, where you can monitor values in real-time.

Pump can also be controlled manually via Blynk App/Web Dashboard.
