#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "ESP32_AP";       // Name of the ESP32 network
const char* password = "12345678";  // Password for the network

const char* server = "http://192.168.4.2:3000/api/water-level";

float waterLevel = 12.0; // Initialize with a default value

void setup() {
  Serial.begin(115200);

  // Create Wi-Fi access point
  Serial.println("Setting up Wi-Fi Access Point...");
  WiFi.softAP(ssid, password);
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());

  delay(1000);
}

void loop() {
    Serial.print("ESP32 AP IP Address");
    Serial.println(WiFi.softAPIP());

    Serial.print("Current Water Level: ");
    Serial.println(waterLevel);

    // Specify content-type header
    HTTPClient http;
    http.begin(server);
    http.addHeader("Content-Type", "application/json");

    // Create JSON payload with dynamic waterLevel
    String payload = "{\"level\": " + String(waterLevel) + "}";

    // Send HTTP POST request
    int httpResponseCode = http.POST(payload);

    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);

    // Get and print the response payload (if any)
    String response = http.getString();
    Serial.println(response);

    http.end();

    // Example of changing waterLevel:
    waterLevel = waterLevel + 0.5; // Increase waterLevel by 0.5 in each loop
    if (waterLevel > 20.0) {
    waterLevel = 12.0; // Reset waterLevel to 12.0 if it exceeds 20.0
    }

    delay(2000); // Send data every 10 seconds
  }

