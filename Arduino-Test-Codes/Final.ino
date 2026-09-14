#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <RTClib.h>
#include <time.h>

// =====================================================
// WiFi
// =====================================================

const char* ssid = "XXXXXX";
const char* password = "XXXXXXXXXXX";

// =====================================================
// Render FastAPI
// =====================================================

const char* serverUrl =
  "https://smartmedbox-3xct.onrender.com/api/sensor";

// =====================================================
// Sensor Pins
// =====================================================

#define REED_PIN 27
#define IR_PIN   26

// =====================================================
// RTC
// =====================================================

RTC_DS3231 rtc;

// =====================================================
// Bangladesh Timezone
// Bangladesh = UTC + 6
// =====================================================

const long GMT_OFFSET_SEC = 6 * 60 * 60;
const int DAYLIGHT_OFFSET_SEC = 0;

// NTP server
const char* ntpServer = "pool.ntp.org";

// =====================================================
// Timing
// =====================================================

unsigned long lastSendTime = 0;
const unsigned long sendInterval = 5000; // 5 seconds

// =====================================================
// FUNCTION DECLARATIONS
// =====================================================

void syncRTCWithNTP();
void sendSensorData(
  DateTime now,
  bool boxOpen,
  bool irDetected
);

// =====================================================
// SETUP
// =====================================================

void setup() {

  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("--------------------------------");
  Serial.println(" SmartMedBox");
  Serial.println(" ESP32 + RTC + Reed + IR + API");
  Serial.println("--------------------------------");

  // ===================================================
  // I2C
  // ===================================================

  Wire.begin(21, 22);

  // ===================================================
  // DS3231
  // ===================================================

  if (!rtc.begin()) {

    Serial.println("ERROR: DS3231 not found!");

    while (1);
  }

  Serial.println("DS3231 detected!");

  // ===================================================
  // Reed Switch
  // ===================================================

  pinMode(REED_PIN, INPUT_PULLUP);

  // ===================================================
  // IR Sensor
  // ===================================================

  pinMode(IR_PIN, INPUT);

  // ===================================================
  // WiFi
  // ===================================================

  Serial.println();
  Serial.print("Connecting to WiFi");

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {

    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi Connected!");

  Serial.print("ESP32 IP: ");
  Serial.println(WiFi.localIP());

  Serial.println("--------------------------------");

  // ===================================================
  // NTP TIME CONFIGURATION
  // ===================================================

  Serial.println("Getting Bangladesh time from NTP...");

  configTime(
    GMT_OFFSET_SEC,
    DAYLIGHT_OFFSET_SEC,
    ntpServer
  );

  // ===================================================
  // SYNC DS3231 WITH NTP
  // ===================================================

  syncRTCWithNTP();

  Serial.println("--------------------------------");
}


// =====================================================
// LOOP
// =====================================================

void loop() {

  // ===================================================
  // 1. READ RTC
  // ===================================================

  DateTime now = rtc.now();

  // ===================================================
  // 2. READ REED SWITCH
  // ===================================================

  int reedState = digitalRead(REED_PIN);

  bool boxOpen;

  if (reedState == LOW) {

    boxOpen = false;

  } else {

    boxOpen = true;
  }

  // ===================================================
  // 3. READ IR SENSOR
  // ===================================================

  int irState = digitalRead(IR_PIN);

  bool irDetected;

  if (irState == LOW) {

    irDetected = true;

  } else {

    irDetected = false;
  }

  // ===================================================
  // SERIAL MONITOR
  // ===================================================

  Serial.println();

  Serial.print("RTC Time: ");

  if (now.hour() < 10)
    Serial.print("0");

  Serial.print(now.hour());
  Serial.print(":");

  if (now.minute() < 10)
    Serial.print("0");

  Serial.print(now.minute());
  Serial.print(":");

  if (now.second() < 10)
    Serial.print("0");

  Serial.println(now.second());

  Serial.print("Box: ");

  if (boxOpen)
    Serial.println("OPEN");
  else
    Serial.println("CLOSED");

  Serial.print("IR Raw: ");
  Serial.println(irState);

  Serial.print("IR Status: ");

  if (irDetected)
    Serial.println("OBJECT DETECTED");
  else
    Serial.println("NO OBJECT");

  Serial.println("--------------------------------");

  // ===================================================
  // SEND TO SERVER EVERY 5 SEC
  // ===================================================

  if (millis() - lastSendTime >= sendInterval) {

    lastSendTime = millis();

    sendSensorData(
      now,
      boxOpen,
      irDetected
    );
  }

  delay(1000);
}


// =====================================================
// SYNC DS3231 WITH NTP
// =====================================================

void syncRTCWithNTP() {

  struct tm timeinfo;

  // Wait for NTP time
  if (!getLocalTime(&timeinfo, 10000)) {

    Serial.println("ERROR: Failed to get NTP time!");

    Serial.println("Using existing DS3231 time.");

    return;
  }

  // ===================================================
  // Print NTP Time
  // ===================================================

  Serial.println();
  Serial.println("NTP Time received:");

  Serial.printf(
    "%04d-%02d-%02d %02d:%02d:%02d\n",
    timeinfo.tm_year + 1900,
    timeinfo.tm_mon + 1,
    timeinfo.tm_mday,
    timeinfo.tm_hour,
    timeinfo.tm_min,
    timeinfo.tm_sec
  );

  // ===================================================
  // Convert NTP time to DateTime
  // ===================================================

  DateTime ntpTime(
    timeinfo.tm_year + 1900,
    timeinfo.tm_mon + 1,
    timeinfo.tm_mday,
    timeinfo.tm_hour,
    timeinfo.tm_min,
    timeinfo.tm_sec
  );

  // ===================================================
  // Set DS3231
  // ===================================================

  rtc.adjust(ntpTime);

  Serial.println("DS3231 synchronized with NTP!");

  // ===================================================
  // Verify RTC
  // ===================================================

  DateTime rtcNow = rtc.now();

  Serial.println();
  Serial.println("RTC Time after synchronization:");

  Serial.printf(
    "%04d-%02d-%02d %02d:%02d:%02d\n",
    rtcNow.year(),
    rtcNow.month(),
    rtcNow.day(),
    rtcNow.hour(),
    rtcNow.minute(),
    rtcNow.second()
  );
}


// =====================================================
// SEND SENSOR DATA TO RENDER
// =====================================================

void sendSensorData(
  DateTime now,
  bool boxOpen,
  bool irDetected
) {

  // ===================================================
  // Check WiFi
  // ===================================================

  if (WiFi.status() != WL_CONNECTED) {

    Serial.println("WiFi disconnected!");
    Serial.println("Trying to reconnect...");

    WiFi.disconnect();
    WiFi.begin(ssid, password);

    return;
  }

  // ===================================================
  // Create RTC String
  // ===================================================

  char rtcTime[25];

  snprintf(
    rtcTime,
    sizeof(rtcTime),
    "%04d-%02d-%02dT%02d:%02d:%02d",
    now.year(),
    now.month(),
    now.day(),
    now.hour(),
    now.minute(),
    now.second()
  );

  // ===================================================
  // Create JSON
  // ===================================================

  String jsonData = "{";

  jsonData += "\"device_id\":\"MEDBOX001\",";

  jsonData += "\"box_open\":";
  jsonData += boxOpen ? "true" : "false";

  jsonData += ",";

  jsonData += "\"ir_detected\":";
  jsonData += irDetected ? "true" : "false";

  jsonData += ",";

  // HX711 এখনো যুক্ত করা হয়নি
  jsonData += "\"weight\":null,";

  jsonData += "\"rtc_time\":\"";
  jsonData += rtcTime;
  jsonData += "\"";

  jsonData += "}";

  // ===================================================
  // HTTP POST
  // ===================================================

  HTTPClient http;

  http.begin(serverUrl);

  http.addHeader(
    "Content-Type",
    "application/json"
  );

  Serial.println();
  Serial.println("Sending sensor data...");
  Serial.println(jsonData);

  int httpResponseCode = http.POST(jsonData);

  // ===================================================
  // Response
  // ===================================================

  Serial.print("HTTP Response Code: ");
  Serial.println(httpResponseCode);

  if (httpResponseCode > 0) {

    String response = http.getString();

    Serial.println("Server Response:");
    Serial.println(response);

  } else {

    Serial.print("HTTP Error: ");
    Serial.println(
      http.errorToString(httpResponseCode)
    );
  }

  http.end();

  Serial.println("--------------------------------");
}
