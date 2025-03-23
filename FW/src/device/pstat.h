#ifndef PSTAT_H
#define PSTAT_H

#include <Arduino.h>

// Initializes the pstat module for SWV operation.
// newGain: gain setting for the LMP91000 (for example, 7 for 350 kΩ)
void pstatInit(uint8_t newGain);

// Runs SWV (Square Wave Voltammetry) using the provided parameters.
// newGain: gain setting, startV: starting voltage (mV), endV: ending voltage (mV),
// pulseAmp: square wave amplitude (mV), stepV: voltage increment (mV),
// freq: square wave frequency (Hz), setToZero: if true, resets outputs at end.
void runSWV(uint8_t newGain, int16_t startV, int16_t endV,
            int16_t pulseAmp, int16_t stepV, double freq, bool setToZero);

// Debug control functions.
void setPstatDebug(bool on);
bool pstatDebugEnabled();

// Data logging functions.
void writeVoltammogramToFile();
void readFileAndSendOverSerial();
void clearVoltammogramFile();
void clearVoltammogramArrays();

// Public helper functions for pstat settings.
void updatePstatGain(uint8_t newGain);
uint8_t getPstatGain();

void updatePstatBias(uint8_t newBias);
uint8_t getPstatBias();

void updateAdcAverages(int newAverage);
int getAdcAverages();

#endif // PSTAT_H
