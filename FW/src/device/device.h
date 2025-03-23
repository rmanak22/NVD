#ifndef DEVICE_H
#define DEVICE_H

#include <Arduino.h>

// Initialization functions
void initHardware();

// LED control functions
void toggleLED();
void setLED(bool on);

// 
void setDebugLevel(bool on);


#endif // DEVICE_H