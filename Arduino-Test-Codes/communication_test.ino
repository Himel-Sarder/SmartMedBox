#include <WiFi.h>
#include <HTTPClient.h>

// ===== WiFi =====
const char* ssid = "Mama";
const char* password = "MamaBari9900";

// ===== Render API =====
const char* serverUrl =
  "https://smartmedbox-3xct.onrender.com/api/sensor";

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("=== ESP32 -> Render API Test ===");

  WiFi.begin(ssid, password);

  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi Connected!");
  Serial.print("ESP32 IP: ");
  Serial.println(WiFi.localIP());

  sendTestData();
}

void loop() {
  // Test-এর জন্য একবারই data পাঠানো হচ্ছে
}

void sendTestData() {

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnected!");
    return;
  }

  HTTPClient http;

  http.begin(serverUrl);
  http.addHeader("Content-Type", "application/json");

  String jsonData = R"({
    "device_id": "MEDBOX001",
    "box_open": false,
    "ir_detected": true,
    "weight": null,
    "rtc_time": "2026-09-12T13:30:00"
  })";

  Serial.println();
  Serial.println("Sending data...");
  Serial.println(jsonData);

  int httpResponseCode = http.POST(jsonData);

  Serial.print("HTTP Response Code: ");
  Serial.println(httpResponseCode);

  if (httpResponseCode > 0) {

    String response = http.getString();

    Serial.println("Server Response:");
    Serial.println(response);

  } else {

    Serial.print("Error sending request: ");
    Serial.println(http.errorToString(httpResponseCode));
  }

  http.end();
}