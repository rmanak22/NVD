#include "device.h"
#include <Wire.h>
#include <SPIFFS.h>
#include "LMP91000.h"  // Include your LMP91000 driver header
#include <WiFi.h>
#include "soc/soc.h"          // Required for WRITE_PERI_REG
#include "soc/rtc_cntl_reg.h" // Required for RTC_CNTL_BROWN_OUT_REG


// Define static variables and constants local to this module.
static bool debugLevel = true; // Default debug level is low (false)

static const int LEDPIN = 26;   // LED pin; adjust as necessary
static const int MENB   = 5;    // Pin to enable the potentiostat

//--------------------------------------------------------//
// Internal (static) initialization functions
//--------------------------------------------------------//

// Initialize the UART for debugging.
static void initUART() {
    Serial.begin(115200);
    while (!Serial) {
        delay(10);
    }
    if (debugLevel) {
        Serial.println("UART initialized.");
    }
}

// Initialize the LED(s).
static void initLEDs() {
    pinMode(LEDPIN, OUTPUT);
    digitalWrite(LEDPIN, LOW);  // start with LED off
    if (debugLevel) {
        Serial.println("LEDs initialized.");
    }
}

// Initialize WIFI AP.
static void initWiFiAP() {
    // Set a fixed IP configuration for the ESP32 AP.
    IPAddress local_IP(192, 168, 4, 1);
    IPAddress gateway(192, 168, 4, 1);
    IPAddress subnet(255, 255, 255, 0);
    WiFi.softAPConfig(local_IP, gateway, subnet);

    // Set the ESP32 into Access Point mode and start the AP.
    WiFi.mode(WIFI_AP);
    WiFi.softAP("metallyze_sensor", "safewater");
    delay(500); // Give the AP time to initialize

    if (debugLevel) {
        Serial.println("WiFi Access Point started successfully.");
        Serial.print("AP IP address: ");
        Serial.println(WiFi.softAPIP());
    }
}


//--------------------------------------------------------//
// Public functions (as declared in device.h)
//--------------------------------------------------------//

void initHardware() {
    // Disable brownout detector to avoid resets during WiFi startup.
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
    if (debugLevel) {
        Serial.println("Brownout detector disabled.");
    }

    initUART();
    if (debugLevel) {
        Serial.println("Starting hardware initialization...");
    }

    // Initialize SPIFFS (for file storage)
    if (!SPIFFS.begin(true)) {
        if (debugLevel) {
            Serial.println("Error mounting SPIFFS");
        }
    } else {
        if (debugLevel) {
            Serial.println("SPIFFS mounted successfully.");
        }
    }

    // Initialize the I2C bus (used by the potentiostat and other I2C devices)
    Wire.begin();
    if (debugLevel) {
        Serial.println("I2C initialized.");
    }

    // Initialize LED(s)
    initLEDs();

    // Initialize WiFi in AP mode.
    initWiFiAP();

    if (debugLevel) {
        Serial.println("All hardware initialized.");
    }
}

void toggleLED() {
    int currentState = digitalRead(LEDPIN);
    digitalWrite(LEDPIN, !currentState);
    if (debugLevel) {
        Serial.print("LED toggled to: ");
        Serial.println(!currentState ? "HIGH" : "LOW");
    }
}

void setLED(bool on) {
    digitalWrite(LEDPIN, on ? HIGH : LOW);
    if (debugLevel) {
        Serial.print("LED set to: ");
        Serial.println(on ? "HIGH" : "LOW");
    }
}

// Function to set the debug level. When debug is enabled, internal messages are printed.
void setDebugLevel(bool on) {
    debugLevel = on;
    if (debugLevel) {
        Serial.println("Debug level set to HIGH.");
    }
}

// New public helper function that returns free heap memory.
unsigned long getFreeHeap() {
    return ESP.getFreeHeap();
}
