#define BLYNK_TEMPLATE_ID "TMPL6x8g6vjz6"
#define BLYNK_TEMPLATE_NAME "Soil Monitor"
#define BLYNK_AUTH_TOKEN "mwA_IBZDhqNMc8IOwvosuXBjuT0uOm9v"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>

// ================== WIFI & BLYNK CREDENTIALS ==================
char ssid[] = "testing";     // Replace with your WiFi network name
char pass[] = "123456789"; // Replace with your WiFi password

// ================== PIN DEFINITIONS ==================
#define DHT_PIN 23           // DHT22 connected to GPIO23
#define DHT_TYPE DHT11       // DHT22 sensor type
#define SOIL_MOISTURE_PIN 34  // Soil moisture sensor to GPIO4 (analog pin)
#define RELAY_PIN 25         // Relay IN pin connected to GPIO25 (OUTPUT capable)
#define LED_PIN 2            // ESP32 built-in LED

// ================== VIRTUAL PINS ==================
#define V_TEMPERATURE V0     // Temperature display
#define V_HUMIDITY V1        // Humidity display
#define V_SOIL_MOISTURE V2   // Soil moisture display
#define V_PUMP_STATUS V3     // Pump status LED
#define V_MANUAL_PUMP V4     // Manual pump control button
#define V_SYSTEM_STATUS V5   // Overall system status
#define V_LAST_WATERING V6   // Time since last watering
#define V_AUTO_MODE V7       // Auto mode switch

// ================== THRESHOLDS ==================
const float MIN_HUMIDITY = 40.0;      // Minimum air humidity (%)
const float MAX_TEMPERATURE = 30.0;   // Maximum temperature (°C)  
const int DRY_SOIL_THRESHOLD = 2500;  // Soil moisture threshold (higher = drier)

// ================== TIMING ==================
unsigned long lastSensorRead = 0;
unsigned long lastPumpActivation = 0;
unsigned long lastBlynkUpdate = 0;
const unsigned long SENSOR_READ_INTERVAL = 3000;    // Read sensors every 3 seconds
const unsigned long PUMP_COOLDOWN = 30000;          // 30 seconds between pump activations
const unsigned long PUMP_DURATION = 3000;           // Pump runs for 3 seconds
const unsigned long BLYNK_UPDATE_INTERVAL = 10000;   // Update Blynk every 10 seconds

// ================== OBJECTS ==================
DHT dht(DHT_PIN, DHT_TYPE);
BlynkTimer timer;

// ================== VARIABLES ==================
float temperature = 0;
float humidity = 0;
int soilMoisture = 0;
bool pumpActive = false;
bool autoMode = true;  // Auto watering mode

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== ESP32 Soil Moisture Control System with Blynk ===");

  // Initialize pins
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  // Relay OFF initially (active LOW)
  digitalWrite(RELAY_PIN, HIGH);  
  digitalWrite(LED_PIN, LOW);

  // Initialize DHT sensor
  dht.begin();

  // Initialize WiFi and Blynk with connection feedback
  Serial.print("🔗 Connecting to WiFi: ");
  Serial.println(ssid);
  
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  
  // Wait for connection with timeout
  int connectionAttempts = 0;
  while (!Blynk.connected() && connectionAttempts < 30) {
    Serial.print(".");
    delay(500);
    connectionAttempts++;
  }
  
  if (Blynk.connected()) {
    Serial.println("\n✅ Connected to Blynk successfully!");
  } else {
    Serial.println("\n❌ Failed to connect to Blynk!");
  }
  
  // Setup timers
  timer.setInterval(SENSOR_READ_INTERVAL, readAndProcessSensors);
  timer.setInterval(BLYNK_UPDATE_INTERVAL, updateBlynk);
  timer.setInterval(1000L, blinkStatusLED);
  timer.setInterval(10000L, checkBlynkConnection); // Check connection every 10s

  delay(2000); // Wait for sensor to stabilize

  Serial.println("System initialized!");
  printSettings();
  Serial.println("Starting monitoring...\n");
  
  // Initialize Blynk widgets with default values
  if (Blynk.connected()) {
    initializeBlynkWidgets();
  }
}

void loop() {
  Blynk.run();
  timer.run();

  // Turn off pump after specified duration
  if (pumpActive && (millis() - lastPumpActivation >= PUMP_DURATION)) {
    deactivatePump();
  }
}

// ================== BLYNK INITIALIZATION FUNCTION ==================

void initializeBlynkWidgets() {
  Serial.println("🔄 Initializing Blynk widgets with default values...");
  
  // Set default sensor values (will be updated once sensors are read)
  Blynk.virtualWrite(V_TEMPERATURE, 25.0);        // Default: 25°C
  Blynk.virtualWrite(V_HUMIDITY, 50.0);           // Default: 50%
  Blynk.virtualWrite(V_SOIL_MOISTURE, 2000);      // Default: 2000 (moderate)
  
  // Set pump status and controls
  Blynk.virtualWrite(V_PUMP_STATUS, 0);           // Pump OFF (LED off)
  Blynk.virtualWrite(V_MANUAL_PUMP, 0);           // Manual button released
  
  // Set mode and status
  Blynk.virtualWrite(V_AUTO_MODE, autoMode);      // Auto mode ON by default
  Blynk.virtualWrite(V_SYSTEM_STATUS, "🔄 System Starting...");
  
  // Set last watering time
  Blynk.virtualWrite(V_LAST_WATERING, "Never");   // No previous watering
  
  Serial.println("✅ Blynk widgets initialized with default values!");
}

// ================== BLYNK VIRTUAL PIN HANDLERS ==================

// Manual pump control switch (momentary activation)
BLYNK_WRITE(V_MANUAL_PUMP) {
  int switchState = param.asInt();
  
  if (switchState == 1 && !pumpActive) {
    if (canActivatePump()) {
      Serial.println("📱 Manual watering triggered from Blynk!");
      activatePump();
      // Auto-reset switch after activation
      delay(100);
      Blynk.virtualWrite(V_MANUAL_PUMP, 0);
    } else {
      Serial.println("⏳ Manual watering blocked - pump in cooldown");
      Blynk.virtualWrite(V_SYSTEM_STATUS, "Pump in cooldown");
      // Reset switch if activation failed
      Blynk.virtualWrite(V_MANUAL_PUMP, 0);
    }
  }
}

// Auto mode toggle switch
BLYNK_WRITE(V_AUTO_MODE) {
  autoMode = param.asInt();
  Serial.println("🔄 Auto mode: " + String(autoMode ? "ON" : "OFF"));
  
  String status = autoMode ? "Auto Mode: ON" : "Auto Mode: OFF";
  Blynk.virtualWrite(V_SYSTEM_STATUS, status);
}

// ================== SENSOR & CONTROL FUNCTIONS ==================

void readAndProcessSensors() {
  readSensors();
  printReadings();

  // Check if watering is needed (only in auto mode)
  if (autoMode && shouldWater() && canActivatePump()) {
    activatePump();
  }
}

void readSensors() {
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();
  soilMoisture = analogRead(SOIL_MOISTURE_PIN);

  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("❌ Error reading DHT22 sensor!");
    humidity = -1;
    temperature = -1;
  }
}

void updateBlynk() {
  // Only update if connected to Blynk
  if (!Blynk.connected()) {
    Serial.println("⚠️ Blynk not connected - skipping update");
    return;
  }

  // Send sensor data to Blynk
  if (temperature > 0) {
    Blynk.virtualWrite(V_TEMPERATURE, temperature);
  } else {
    Blynk.virtualWrite(V_TEMPERATURE, 0); // Send 0 for error
  }
  
  if (humidity > 0) {
    Blynk.virtualWrite(V_HUMIDITY, humidity);
  } else {
    Blynk.virtualWrite(V_HUMIDITY, 0); // Send 0 for error
  }
  
  Blynk.virtualWrite(V_SOIL_MOISTURE, soilMoisture);
  
  // Update pump status LED
  Blynk.virtualWrite(V_PUMP_STATUS, pumpActive ? 255 : 0);
  
  // Calculate time since last watering
  if (lastPumpActivation > 0) {
    unsigned long timeSinceWatering = (millis() - lastPumpActivation) / 1000;
    String timeStr = formatTime(timeSinceWatering);
    Blynk.virtualWrite(V_LAST_WATERING, timeStr);
  } else {
    Blynk.virtualWrite(V_LAST_WATERING, "Never");
  }
  
  // Update system status
  String systemStatus = getSystemStatus();
  Blynk.virtualWrite(V_SYSTEM_STATUS, systemStatus);
  
  Serial.println("📡 Blynk data updated: " + systemStatus);
}

// Check and maintain Blynk connection
void checkBlynkConnection() {
  if (!Blynk.connected()) {
    Serial.println("🔄 Reconnecting to Blynk...");
    Blynk.connect();
    delay(1000);
    
    if (Blynk.connected()) {
      Serial.println("✅ Reconnected to Blynk!");
      initializeBlynkWidgets(); // Reinitialize widgets after reconnection
    } else {
      Serial.println("❌ Blynk reconnection failed");
    }
  }
}

String formatTime(unsigned long seconds) {
  if (seconds < 60) {
    return String(seconds) + "s ago";
  } else if (seconds < 3600) {
    return String(seconds / 60) + "m ago";
  } else {
    unsigned long hours = seconds / 3600;
    unsigned long minutes = (seconds % 3600) / 60;
    return String(hours) + "h " + String(minutes) + "m ago";
  }
}

String getSystemStatus() {
  if (pumpActive) {
    return "🟢 WATERING NOW";
  } else if (!autoMode) {
    return "🔵 MANUAL MODE";
  } else if (shouldWater() && !canActivatePump()) {
    return "🟡 COOLDOWN";
  } else if (shouldWater()) {
    return "🟠 NEEDS WATER";
  } else {
    return "🟢 ALL GOOD";
  }
}

bool shouldWater() {
  bool soilIsDry = (soilMoisture > DRY_SOIL_THRESHOLD);
  bool humidityLow = (humidity > 0) && (humidity < MIN_HUMIDITY);
  bool temperatureHigh = (temperature > 0) && (temperature > MAX_TEMPERATURE);

  return soilIsDry && (humidityLow || temperatureHigh);
}

bool canActivatePump() {
  return !pumpActive && (millis() - lastPumpActivation >= PUMP_COOLDOWN);
}

void activatePump() {
  digitalWrite(RELAY_PIN, LOW);  // Relay ON (active LOW)
  pumpActive = true;
  lastPumpActivation = millis();

  Serial.println("\n💧💧💧 WATERING STARTED! 💧💧💧");
  Serial.println("Conditions that triggered watering:");

  if (soilMoisture > DRY_SOIL_THRESHOLD) {
    Serial.println("  🌱 Soil is dry (" + String(soilMoisture) + ")");
  }
  if (humidity < MIN_HUMIDITY && humidity > 0) {
    Serial.println("  💨 Low humidity (" + String(humidity) + "%)");
  }
  if (temperature > MAX_TEMPERATURE && temperature > 0) {
    Serial.println("  🌡 High temperature (" + String(temperature) + "°C)");
  }

  Serial.println("Watering for " + String(PUMP_DURATION / 1000) + " seconds...");
  digitalWrite(LED_PIN, HIGH); // Solid LED during watering
  
  // Notify Blynk
  Blynk.logEvent("watering_started", "Plant watering started!");
}

void deactivatePump() {
  digitalWrite(RELAY_PIN, HIGH);   // Relay OFF (active LOW)
  pumpActive = false;
  digitalWrite(LED_PIN, LOW);

  Serial.println("💧 Watering completed!");
  Serial.println("Next watering available in " + String(PUMP_COOLDOWN / 1000) + " seconds\n");
  
  // Notify Blynk
  Blynk.logEvent("watering_completed", "Plant watering completed!");
}

void printReadings() {
  Serial.println("==========================================");
  Serial.println("📊 SENSOR READINGS:");

  // Temperature
  if (temperature > 0) {
    Serial.print("🌡  Temperature: " + String(temperature, 1) + "°C");
    if (temperature > MAX_TEMPERATURE) Serial.println(" ⚠ HIGH!");
    else Serial.println(" ✅");
  } else {
    Serial.println("🌡  Temperature: ERROR");
  }

  // Humidity  
  if (humidity > 0) {
    Serial.print("💨 Humidity: " + String(humidity, 1) + "%");
    if (humidity < MIN_HUMIDITY) Serial.println(" ⚠ LOW!");
    else Serial.println(" ✅");
  } else {
    Serial.println("💨 Humidity: ERROR");
  }

  // Soil Moisture
  Serial.print("🌱 Soil Moisture: " + String(soilMoisture));
  if (soilMoisture > DRY_SOIL_THRESHOLD) Serial.println(" ⚠ DRY!");
  else Serial.println(" ✅ WET");

  // Pump Status
  Serial.println("💧 Pump Status: " + String(pumpActive ? "ACTIVE 🟢" : "INACTIVE 🔴"));

  // Overall Assessment
  if (shouldWater() && autoMode) {
    Serial.println("🚨 WATERING NEEDED!");
    if (!canActivatePump()) Serial.println("⏳ Pump in cooldown period...");
  } else if (!autoMode) {
    Serial.println("🔵 Manual mode - auto watering disabled");
  } else {
    Serial.println("😊 All conditions are good!");
  }

  Serial.println("==========================================\n");
}

void printSettings() {
  Serial.println("\n⚙ SYSTEM SETTINGS:");
  Serial.println("📏 Thresholds:");
  Serial.println("  - Max Temperature: " + String(MAX_TEMPERATURE) + "°C");
  Serial.println("  - Min Humidity: " + String(MIN_HUMIDITY) + "%");
  Serial.println("  - Dry Soil Level: " + String(DRY_SOIL_THRESHOLD));
  Serial.println("⏱ Timing:");
  Serial.println("  - Sensor Reading: Every " + String(SENSOR_READ_INTERVAL / 1000) + " seconds");
  Serial.println("  - Pump Duration: " + String(PUMP_DURATION / 1000) + " seconds");
  Serial.println("  - Pump Cooldown: " + String(PUMP_COOLDOWN / 1000) + " seconds");
  Serial.println("🔌 Pin Configuration:");
  Serial.println("  - DHT22: GPIO" + String(DHT_PIN));
  Serial.println("  - Soil Sensor: GPIO" + String(SOIL_MOISTURE_PIN));
  Serial.println("  - Relay: GPIO" + String(RELAY_PIN));
  Serial.println("==========================================");
}

void blinkStatusLED() {
  static bool ledState = false;

  if (!pumpActive) {
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState);
  }
}