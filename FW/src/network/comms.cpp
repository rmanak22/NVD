#include "comms.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <SPIFFS.h>

bool sendCSVData(const char* serverURL) {
    // Ensure SPIFFS is mounted.
    if (!SPIFFS.begin(true)) {
        Serial.println("Failed to mount SPIFFS");
        return false;
    }

    File file = SPIFFS.open("/data.csv", FILE_READ);
    if (!file) {
        Serial.println("Failed to open CSV file for reading");
        return false;
    }

    // Read entire CSV file into a String.
    String payload;
    while (file.available()) {
        payload += char(file.read());
    }
    file.close();
    Serial.print("Payload to Send:");
    Serial.println(payload);

    HTTPClient http;
    http.begin(serverURL);
    http.addHeader("Content-Type", "text/csv");

    int httpResponseCode = http.POST(payload);
    if (httpResponseCode > 0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        Serial.println("Response payload:");
        Serial.println(http.getString());
    } else {
        Serial.print("Error sending POST: ");
        Serial.println(httpResponseCode);
    }
    http.end();
    return (httpResponseCode > 0);
}

