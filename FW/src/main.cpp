#include <Arduino.h>
#include "device/device.h"

// We'll use a simple timer to trigger our LED functions every 1 second.
unsigned long previousMillis = 0;
const unsigned long interval = 1000; // 1 second

// For demonstration, we'll alternate between toggling the LED and setting it explicitly.
bool toggleMode = true;

void setup() {
    initHardware();
    Serial.println("Setup complete. Entering main loop...");
}

void loop() {
    unsigned long currentMillis = millis();

    // Check if 1 second has elapsed
    if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;

        if (toggleMode) {
            // Call the toggle function
            toggleLED();
            Serial.println("Test");
        }
        else {
            // Call the setLED function to turn the LED ON (change to false to turn it off)
            setLED(true);
        }

        // For demonstration, alternate mode each second.
        toggleMode = !toggleMode;
    }

}
