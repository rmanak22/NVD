#ifndef DEVICE_H
#define DEVICE_H

#include <Arduino.h>

// Initialization functions
void initHardware();

// LED control functions
void toggleLED();
void setLED(bool on);

// Debug level control function
void setDebugLevel(bool on);

// New helper function to get free heap memory (RAM)
unsigned long getFreeHeap();

#endif // DEVICE_H
