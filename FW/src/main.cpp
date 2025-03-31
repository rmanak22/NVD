#include <Arduino.h>
#include "device/device.h"   // Contains initHardware, toggleLED, setLED, getFreeHeap, etc.
#include "device/pstat.h"    // Contains pstatInit, runSWV, data logging and helper functions
#include "network/comms.h"   // Contains sendCSVData

#include <WebServer.h>

// Create a web server on port 80.
WebServer server(80);

// Global flag to indicate a sweep is requested.
volatile bool sweepRequested = false;

// HTTP GET handler for "/sweep"
void handleSweepRequest() {
    sweepRequested = true;
    server.send(200, "text/plain", "Sweep requested");
}

void setup() {
    // Initialize device hardware.
    initHardware();

    // Optionally enable debug for device and pstat modules.
    setDebugLevel(true);
    setPstatDebug(true);

    // Initialize the HTTP server and register the sweep handler.
    server.on("/sweep", HTTP_GET, handleSweepRequest);
    server.begin();

    Serial.println("System initialized. Waiting for sweep request...");
}

void loop() {
    // Process any incoming HTTP requests.
    server.handleClient();
    static unsigned long lastSweepTime = 0U;
    static const unsigned long sweepInterval = 3500U;

    // Set sweepRequested every 10 seconds without blocking.
    if (millis() - lastSweepTime >= sweepInterval) {
        sweepRequested = true;
    }

    // Only run the sweep if the flag is set by the HTTP request.
    if (sweepRequested) {
        // Reset the flag.
        sweepRequested = false;

        // --- Step 1: Clear Data Memory ---
        Serial.println("Clearing data memory...");
        clearVoltammogramFile();
        clearVoltammogramArrays();
        Serial.println("Data memory cleared.");

        // --- Step 2: Log Memory Usage ---
        unsigned long freeHeap = getFreeHeap();
        Serial.print("Free heap: ");
        Serial.println(freeHeap);

        // --- Step 3: Run SWV Sweep ---
        Serial.println("Starting SWV sweep...");

        // Print current pstat settings.
        Serial.print("Pstat Gain: ");
        Serial.println(getPstatGain());
        Serial.print("Pstat Bias: ");
        Serial.println(getPstatBias());
        Serial.print("ADC Averages: ");
        Serial.println(getAdcAverages());
        Serial.println("SWV Settings: Gain=7, StartV=-200 mV, EndV=200 mV, PulseAmp=20 mV, StepV=5 mV, Freq=10 Hz, SetToZero=true");

        runSWV(7, -200, 200, 20, 5, 10.0, true);
        Serial.println("SWV sweep complete.");

        // --- Step 4: Write and Send Data Over Serial ---
        Serial.println("Writing logged data to file...");
        writeVoltammogramToFile();
        Serial.println("Data written to /data.csv.");

        Serial.println("Sending logged data over Serial...");
        readFileAndSendOverSerial();
        Serial.println("\nData sent over Serial.");

        // --- Step 5: Optionally, Send Data Over HTTP ---
        Serial.println("Sending CSV data to backend...");
        const char* backendURL = "http://192.168.4.2:80/upload";  // Change to your computer's IP
        if (sendCSVData(backendURL)) {
            Serial.println("CSV data sent successfully.");
        } else {
            Serial.println("Failed to send CSV data.");
        }

        // --- Step 6: Log Memory Usage ---
        freeHeap = getFreeHeap();
        Serial.print("Free heap: ");
        Serial.println(freeHeap);

        Serial.println("Sweep cycle complete. Waiting for next sweep request...");

        // Update time of last sweep
        lastSweepTime = millis();
    }
}
