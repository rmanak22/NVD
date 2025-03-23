#include <Arduino.h>
#include "device/device.h"   // Contains initHardware, toggleLED, setLED, etc.
#include "device/pstat.h"    // Contains pstatInit, runSWV, data logging and helper functions

void setup() {
    // Initialize device hardware
    initHardware();

    // Optionally enable pstat debug output
    setPstatDebug(true);

    Serial.println("System initialized. Beginning main cycle...");
}

void loop() {
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

    // Print current pstat settings
    Serial.print("Pstat Gain: ");
    Serial.println(getPstatGain());
    Serial.print("Pstat Bias: ");
    Serial.println(getPstatBias());
    Serial.print("ADC Averages: ");
    Serial.println(getAdcAverages());

    // Print SWV sweep settings (example values)
    Serial.println("SWV Settings: Gain=7, StartV=-200 mV, EndV=200 mV, PulseAmp=20 mV, StepV=5 mV, Freq=10 Hz, SetToZero=true");

    runSWV(7, -200, 200, 20, 5, 10.0, true);
    Serial.println("SWV sweep complete.");

    // --- Step 4: Send Data Over Serial ---
    Serial.println("Sending logged data over Serial...");
    readFileAndSendOverSerial();
    Serial.println("\nData sent over Serial.");

    // --- Step 5: Log Memory Usage ---
    freeHeap = getFreeHeap();
    Serial.print("Free heap: ");
    Serial.println(freeHeap);

    // --- End of Cycle ---
    Serial.println("Cycle complete. Waiting 5 seconds before next cycle...");
    delay(5000);
}
