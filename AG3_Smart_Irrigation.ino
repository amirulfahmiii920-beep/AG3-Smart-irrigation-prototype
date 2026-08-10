#define BLYNK_TEMPLATE_ID "YOUR_BLYNK_TEMPLATE_ID"
#define BLYNK_TEMPLATE_NAME "YOUR_BLYNK_TEMPLATE_NAME"
#define BLYNK_AUTH_TOKEN "YOUR_BLYNK_AUTH_TOKEN"
#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include "DHT.h"
#include <HTTPClient.h>       // NEW: For Telegram
#include <WiFiClientSecure.h> // NEW: For Secure Connection
#include "weather.h"

// --- 1. CREDENTIALS ---
// WiFi & Blynk
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "YOUR_WIFI_CREDENTIALS";
char pass[] = "WIFI PASSWORD";

// Telegram
String BOT_TOKEN = "YOUR_TELEGRAM_TOKEN"; // <--- PASTE TOKEN HERE
String CHAT_ID = "-YOUR_TELEGRAM_ID";     // <--- PASTE ID HERE

// --- 2. PIN DEFINITIONS ---
const int PUMP_VASES[3] = {25, 26, 27}; 
const int SOIL_PINS[3]  = {34, 35, 32}; 

#define TRIG_CLEAN 14
#define ECHO_CLEAN 22
#define PUMP_CLEAN 19 

#define TRIG_RAIN  5
#define ECHO_RAIN  18
#define PUMP_RAIN  21 

#define DHT_PIN 15
#define DHTTYPE DHT11

// --- 3. SETTINGS & THRESHOLDS ---
const int DRY_THRESHOLDS[3] = {30, 30, 30}; 
const int TANK_EMPTY_THRESHOLD = 6; // cm (Big distance = Empty)
const int TANK_FULL_THRESHOLD  = 3;  // cm (Small distance = Full)

const unsigned long PUMP_DURATION = 1000;   // 5 Seconds Pumping
const unsigned long SOAK_COOLDOWN = 5000;   // 5 Seconds Wait (Testing Mode)

// --- 4. GLOBAL VARIABLES ---
DHT dht(DHT_PIN, DHTTYPE);
BlynkTimer timer;

int startMoisture[3] = {0, 0, 0}; 
int moisture[3] = {0, 0, 0};
float humidity[3] = {0, 0, 0};
int levelClean = 0;
int levelRain = 0;
String lastWeatherPrediction = "Unknown";
unsigned long lastPredictionTime = 0;

// State Tracking
unsigned long pumpStartTime[3] = {0, 0, 0}; 
unsigned long lastWaterTime[3] = {0, 0, 0}; 
bool isAutoPumping[3] = {false, false, false};

// Alert Flags (To prevent spamming Telegram)
bool alertSentCleanTank = false;
bool alertSentRainTank = false;

// Manual Control
bool manualOverride[3] = {false, false, false}; // V12, V13, V14
bool manualRainPump = false; // V15

// --- 5. BLYNK BUTTONS ---
BLYNK_WRITE(V12) { manualOverride[0] = param.asInt(); }
BLYNK_WRITE(V13) { manualOverride[1] = param.asInt(); }
BLYNK_WRITE(V14) { manualOverride[2] = param.asInt(); }
BLYNK_WRITE(V15) { manualRainPump = param.asInt(); }

// --- 6. HELPER FUNCTIONS ---

// A. Ultrasonic Reader
// IMPROVED ULTRASONIC FUNCTION (Stable)
int readUltrasonic(int trig, int echo) {
  long totalDuration = 0;
  int validReadings = 0;

  // Take 5 readings rapidly
  for (int i = 0; i < 5; i++) {
      digitalWrite(trig, LOW); delayMicroseconds(2);
      digitalWrite(trig, HIGH); delayMicroseconds(10);
      digitalWrite(trig, LOW);
      
      long duration = pulseIn(echo, HIGH, 25000); // Timeout after 25ms (approx 4 meters)
      
      if (duration > 0) {
          totalDuration += duration;
          validReadings++;
      }
      delay(5); // Tiny wait between shots
  }

  if (validReadings == 0) return 999; // Error: No reading

  long averageDuration = totalDuration / validReadings;
  int distance = averageDuration * 0.034 / 2;
  
  // Filter out "Condensation Noise" (Ignore anything < 2cm)
  if (distance < 1) return 100; // Pretend it's empty if it reads error
  
  return distance;
}

// B. Send Telegram Message (DEBUG VERSION)
void sendTelegram(String text) {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClientSecure client;
    client.setInsecure(); // Skip certificate check
    HTTPClient https;
    
    // Create the URL
    String url = "YOUR_TELEGRAM_API_URL" + BOT_TOKEN + "/sendMessage?chat_id=" + CHAT_ID + "&text=" + text;
    
    // Send Request
    if (https.begin(client, url)) {
      int httpCode = https.GET();  // Get the real status code
      String payload = https.getString(); // Get the error message from Telegram
      
      if (httpCode == 200) {
        Serial.println("SUCCESS: Telegram Sent (" + text + ")");
      } else {
        Serial.println("ERROR: Telegram Rejected Message!");
        Serial.print("   Code: "); Serial.println(httpCode);
        Serial.print("   Reason: "); Serial.println(payload);
      }
      https.end();
    } else {
      Serial.println("Connection Failed (WiFi issue)");
    }
  }
}

void runSystemLogic() {
  
  // A. READ TANKS
  levelClean = readUltrasonic(TRIG_CLEAN, ECHO_CLEAN);
  levelRain  = readUltrasonic(TRIG_RAIN, ECHO_RAIN);
  
  Blynk.virtualWrite(V7, levelClean);
  Blynk.virtualWrite(V8, levelRain);

  // --- TELEGRAM ALERT: CLEAN TANK EMPTY ---
  if (levelClean >= TANK_EMPTY_THRESHOLD) {
      if (!alertSentCleanTank) { // Only send if we haven't sent it yet
          sendTelegram("ALERT: Clean Water Tank is EMPTY! Please refill.");
          Blynk.virtualWrite(V10, "ALERT: Clean Tank Empty!");
          alertSentCleanTank = true; // Lock it so we don't spam
      }
  } else {
      alertSentCleanTank = false; // Reset lock when tank is filled
  }

  // --- TELEGRAM ALERT: RAIN TANK FULL ---
  if (levelRain < TANK_FULL_THRESHOLD) {
      if (!alertSentRainTank) {
          sendTelegram("ALERT: Rain Tank is FULL! Auto-draining started.");
          alertSentRainTank = true;
      }
  } else {
      alertSentRainTank = false;
  }
  
  // B. HYBRID RAIN PUMP
  
  // 1. Priority: Manual Override (Human is Boss)
  if (manualRainPump == true) {
      // WARNING: Manual mode ignores tank levels! Be careful!
      digitalWrite(PUMP_RAIN, LOW); 
  } 
  
  // 2. Priority: Auto Transfer (Smart Mode)
  // Logic: "Rain Tank is FULL" AND "Clean Tank has SPACE"
  else if (levelRain < TANK_FULL_THRESHOLD && levelClean > TANK_FULL_THRESHOLD) { 
      digitalWrite(PUMP_RAIN, LOW); // Safe to Pump
  } 
  
  // 3. Stop (Safety)
  else {
      digitalWrite(PUMP_RAIN, HIGH); // OFF
      
      // If we stopped because Clean Tank is full, you might want to know
      if (levelRain < TANK_FULL_THRESHOLD && levelClean <= TANK_FULL_THRESHOLD) {
         // This means Rain Tank is full, but we CAN'T pump because Clean Tank is also full.
         // Ideally, your Rain Tank should have a physical overflow pipe for this scenario.
      }
  }
  
  // C. VASE SYSTEM
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  if (isnan(h)) h = 0; 
  if (isnan(t)) t = 0;

  // --- START OF AI CHECK ---
  // 1. Send Sensor Data to Blynk
  Blynk.virtualWrite(V4, h);
  Blynk.virtualWrite(V9, t);

  // 2. Ask the AI for a Prediction (Every 15 mins)
  if (millis() - lastPredictionTime > 900000 || lastWeatherPrediction == "Unknown") {
      
      // Call the function from weather.h
      String forecast = predictWeatherAI(t, h);
      
      // Only send Telegram if the weather CHANGED
      if (forecast != lastWeatherPrediction) {
          sendTelegram("Weather Forecast Update: " + forecast);
          Blynk.virtualWrite(V11, forecast); // Update Label Widget on V11
          lastWeatherPrediction = forecast;
      }
      lastPredictionTime = millis();
  }
  // --- END OF AI CHECK ---

  bool cleanHasWater = (levelClean < TANK_EMPTY_THRESHOLD);

  for (int i = 0; i < 3; i++) {
    
    int raw = analogRead(SOIL_PINS[i]);
    moisture[i] = map(raw, 4095, 0, 0, 100);
    moisture[i] = constrain(moisture[i], 0, 100);
    humidity[i] = h;

    // Control Logic
    if (manualOverride[i] == true) {
       // Manual Mode
       if (cleanHasWater) {
          digitalWrite(PUMP_VASES[i], LOW); 
          isAutoPumping[i] = false; 
       } else {
          digitalWrite(PUMP_VASES[i], HIGH);
       }
    }
    else {
       // Auto Mode
       if (!isAutoPumping[i]) {
           if (moisture[i] < DRY_THRESHOLDS[i] && cleanHasWater && (millis() - lastWaterTime[i] > SOAK_COOLDOWN)) {
               digitalWrite(PUMP_VASES[i], LOW); 
               isAutoPumping[i] = true;
               pumpStartTime[i] = millis();
               startMoisture[i] = moisture[i]; // Save Start Value
               
               sendTelegram("Vase " + String(i+1) + ": Watering Started (Dry: " + String(moisture[i]) + "%)");
           } else {
               digitalWrite(PUMP_VASES[i], HIGH);
           }
       }
       else if (isAutoPumping[i]) {
           if (millis() - pumpStartTime[i] >= PUMP_DURATION) {
               digitalWrite(PUMP_VASES[i], HIGH); 
               isAutoPumping[i] = false;
               lastWaterTime[i] = millis();
               
               // --- LEAK DETECTION & TELEGRAM ---
               int moistureChange = moisture[i] - startMoisture[i];
               
               if (moistureChange < 5) {
                   String msg = "FAIL: Vase " + String(i+1) + " Pump ran but soil didn't get wet! Check leak.";
                   sendTelegram(msg);
                   Blynk.virtualWrite(V10, msg);
               } else {
                   String msg = "SUCCESS: Vase " + String(i+1) + " Watered. (+ " + String(moistureChange) + "%)";
                   sendTelegram(msg);
                   Blynk.virtualWrite(V10, "Watering Successful!");
               }
           }
       }
    }
  }

  // D. UPDATE DASHBOARD
  Blynk.virtualWrite(V1, moisture[0]);
  Blynk.virtualWrite(V2, moisture[1]);
  Blynk.virtualWrite(V3, moisture[2]);
  Blynk.virtualWrite(V4, h);
  Blynk.virtualWrite(V5, h); 
  Blynk.virtualWrite(V6, h);
  Blynk.virtualWrite(V9, t);
}

// --- 8. SETUP ---
void setup() {
  Serial.begin(115200);
  dht.begin();
  
  pinMode(TRIG_CLEAN, OUTPUT); pinMode(ECHO_CLEAN, INPUT);
  pinMode(TRIG_RAIN, OUTPUT);  pinMode(ECHO_RAIN, INPUT);
  pinMode(PUMP_CLEAN, OUTPUT); digitalWrite(PUMP_CLEAN, HIGH);
  pinMode(PUMP_RAIN, OUTPUT);  digitalWrite(PUMP_RAIN, HIGH);

  for (int i = 0; i < 3; i++) {
    pinMode(SOIL_PINS[i], INPUT);
    pinMode(PUMP_VASES[i], OUTPUT);
    digitalWrite(PUMP_VASES[i], HIGH); 
  }

  Blynk.begin(auth, ssid, pass);
  
  // Send a startup message to test Telegram
  sendTelegram("System Online: Plant Monitor is Ready!");

  timer.setInterval(1000L, runSystemLogic);
}

// --- 9. LOOP ---
void loop() {
  Blynk.run();
  timer.run();
}