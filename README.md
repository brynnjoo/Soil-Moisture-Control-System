# 🌱 ESP32 Soil Moisture Control System with Blynk Cloud

Proyek ini adalah **sistem kontrol kelembaban tanah otomatis** menggunakan **ESP32**, **sensor soil moisture**, **sensor DHT11**, dan **relay untuk pompa air**.  
Sistem dapat memonitor suhu, kelembaban udara, kelembaban tanah, serta mengontrol pompa secara otomatis maupun manual melalui **Blynk IoT Cloud**.

---

## ⚡ Fitur
- Membaca data **suhu** (°C) dan **kelembaban udara** (%) dari sensor DHT.
- Membaca **kelembaban tanah** menggunakan soil moisture sensor.
- Mengontrol **relay pompa air** secara otomatis berdasarkan kondisi:
  - Tanah kering
  - Kelembaban udara rendah
  - Suhu terlalu tinggi
- **Kontrol manual** pompa melalui aplikasi Blynk.
- Monitoring realtime melalui **Blynk Cloud (App & Web Dashboard)**.
- LED indikator status (berkedip saat idle, menyala solid saat pompa aktif).

---

## 🛠️ Hardware yang Digunakan
- ESP32 Development Board
- DHT11 atau DHT22 (Sensor Suhu & Kelembaban Udara)
- Soil Moisture Sensor (Analog)
- Relay 5V
- Pompa Air Mini (opsional)
- Breadboard, jumper wire, dan power supply

---

## ⚙️ Konfigurasi Pin
| Komponen          | Pin ESP32 |
|-------------------|-----------|
| DHT11             | GPIO 23   |
| Soil Moisture     | GPIO 34    |
| Relay             | GPIO 25   |
| LED (Onboard)     | GPIO 2    |

---

## 📲 Blynk Virtual Pin Mapping
| Virtual Pin |                  Data/Control                  |
|-------------|------------------------------------------------|
| V0          | Temperature (°C)                               |
| V1          | Humidity (%)                                   |
| V2          | Soil Moisture (ADC)                            |
| V3          | Pump Status (ON/OFF)                           |
| V4          | Manual Pump Control (Button / Switch)          |
| V5          | System Status (String text → Label Widget)     |
| V6          | Last Watering Time (Text/Label)                |
| V7          | Auto Mode Switch (Toggle between Auto/Manual)  |
