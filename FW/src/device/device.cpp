#include "device.h"
#include <Wire.h>
#include <SPIFFS.h>
#include "LMP91000.h"  // Include your LMP91000 driver header

// Define static variables and constants local to this module.
static bool debugLevel = false; // Default debug level is low (false)

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

//--------------------------------------------------------//
// Public functions (as declared in device.h)
//--------------------------------------------------------//

void initHardware() {
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
