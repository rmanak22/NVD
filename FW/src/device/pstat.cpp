#include "pstat.h"
#include <Wire.h>
#include <SPIFFS.h>
#include <math.h>
#include "LMP91000.h"

//────────────────────────────────────────────────────────────
// Static Variables and Constants (Private to this module)
//────────────────────────────────────────────────────────────
static bool debugLevel = false; // Default debug off

static const uint16_t opVolt = 3300;       // Operating voltage (mV)
static const uint8_t adcBits = 12;         // ADC resolution (bits)
static const uint16_t dacResolution = 255; // 8-bit DAC: 2^8 - 1

// Pin definitions (adjust as needed)
static const uint8_t dacPin   = 25;    // DAC output pin for Vref
static const uint8_t MENB     = 5;     // Enable pin for LMP91000
static const uint8_t LMP_ADC  = 35;    // ADC pin for reading LMP91000 Vout
static const int LEDPIN       = 26;    // LED pin (for data-point signaling)

// Calibration coefficients (hard-coded defaults)
static float a_coeff = -146.63;
static float b_coeff = 7.64;

// Note: Do not redefine TIA_BIAS, NUM_TIA_BIAS, and TIA_GAIN here,
// since they are defined in LMP91000.h

// Storage for SWV data
static const uint16_t arr_samples = 5000;
static uint16_t arr_cur_index = 0;
static int16_t volts[arr_samples] = {0};
static float amps[arr_samples] = {0};
static int32_t time_Voltammaogram[arr_samples] = {0};
static int number_of_valid_points_in_volts_amps_array = 0;

// Timing variable used in sampling
static unsigned long lastTime = 0;

// DAC output variable (in mV)
static uint16_t dacVout = 1500;

// Number of ADC readings to average
static int num_adc_readings_to_average = 1;

// Global bias setting index.
static uint8_t bias_setting = 0;

// Global LMP gain (default 7)
static uint8_t LMPgainGLOBAL = 7;

// Create a global LMP91000 instance.
static LMP91000 pStat;

//────────────────────────────────────────────────────────────
// Internal Helper Functions (Private, Static)
//────────────────────────────────────────────────────────────

static void pulseLED_on_off(int pin, int on_duration_ms) {
    digitalWrite(pin, HIGH);
    delay(on_duration_ms);
    digitalWrite(pin, LOW);
}

static inline uint16_t convertDACVoutToDACVal(uint16_t dacVout) {
    return dacVout * ((float)dacResolution / opVolt);
}

static inline float analog_read_avg(int num_points, int pin_num) {
    float sum = 0;
    for (int i = 0; i < num_points; i++) {
        sum += analogRead(pin_num);
    }
    return sum / num_points;
}

static void setLMPBias(int16_t voltage) {
    if (voltage < 0)
        pStat.setNegBias();
    else if (voltage > 0)
        pStat.setPosBias();
}

static void setOutputsToZero() {
    dacWrite(dacPin, 0);
    pStat.setBias(0);
}

static inline void reset_Voltammogram_arrays() {
    for (uint16_t i = 0; i < arr_samples; i++) {
        volts[i] = 0;
        amps[i] = 0;
        time_Voltammaogram[i] = 0;
    }
}

static inline void saveVoltammogram(float voltage, float current) {
    volts[arr_cur_index] = (int16_t)voltage;
    amps[arr_cur_index] = current;
    time_Voltammaogram[arr_cur_index] = millis();
    arr_cur_index++;
    number_of_valid_points_in_volts_amps_array++;
}

static void setVoltage(int16_t voltage) {
    const uint16_t minDACVoltage = 1520;
    dacVout = minDACVoltage;
    bias_setting = 0;

    int16_t setV = dacVout * TIA_BIAS[bias_setting];
    voltage = abs(voltage);

    if (voltage < 15) {
        if (voltage > 7.5)
            voltage = 15;
        else {
            bias_setting = 0;
            setV = 0;
            dacVout = 1500;
        }
    }

    if (voltage >= 15) {
        const float v_tolerance = 0.008;
        while (setV > voltage * (1 + v_tolerance) || setV < voltage * (1 - v_tolerance)) {
            if (bias_setting == 0)
                bias_setting = 1;
            dacVout = voltage / TIA_BIAS[bias_setting];
            if (dacVout > opVolt) {
                bias_setting++;
                dacVout = 1500;
                if (bias_setting > NUM_TIA_BIAS)
                    bias_setting = 0;
            }
            setV = dacVout * TIA_BIAS[bias_setting];
        }
    }

    pStat.setBias(bias_setting);
    dacWrite(dacPin, convertDACVoutToDACVal(dacVout));
}

static float biasAndSample(int16_t voltage, uint32_t rate) {
    setLMPBias(voltage);
    setVoltage(voltage);

    if (rate > 10)
        pulseLED_on_off(LEDPIN, 10);
    else
        pulseLED_on_off(LEDPIN, 1);

    while (millis() - lastTime < rate) { }

    int adc_bits = analog_read_avg(num_adc_readings_to_average, LMP_ADC);

    float a = a_coeff;
    float b = b_coeff;
    float v1 = (3.3 / 255.0) * (1 / (2.0 * b)) * (float)adc_bits - (a / (2.0 * b)) * (3.3 / 255.0);
    v1 = v1 * 1000;
    float v2 = dacVout * 0.5;

    float current = 0;
    if (LMPgainGLOBAL == 0)
        current = (((v1 - v2) / 1000) / 1e6) * pow(10, 9);
    else
        current = (((v1 - v2) / 1000) / TIA_GAIN[LMPgainGLOBAL - 1]) * pow(10, 6);

    if (debugLevel) {
        Serial.print(millis());
        Serial.print("\tDesired V: ");
        Serial.print(voltage);
        Serial.print("\tSet V: ");
        Serial.print(dacVout * TIA_BIAS[bias_setting]);
        Serial.print("\tDAC: ");
        Serial.print(dacVout);
        Serial.print("\tADC: ");
        Serial.print(adc_bits);
        Serial.print("\tVout: ");
        Serial.print(v1);
        Serial.print("\tZero: ");
        Serial.print(v2);
        Serial.print("\tI: ");
        Serial.println(current, 4);
    }

    lastTime = millis();
    return current;
}

//────────────────────────────────────────────────────────────
// SWV Functions (Internal: Static Forward and Backward)
//────────────────────────────────────────────────────────────

static void runSWVForward(int16_t startV, int16_t endV, int16_t pulseAmp, int16_t stepV, uint16_t delay_ms) {
    float i_forward = 0;
    float i_backward = 0;
    for (int16_t j = startV; j <= endV; j += stepV) {
        i_forward = biasAndSample(j + pulseAmp, delay_ms);
        i_backward = biasAndSample(j - pulseAmp, delay_ms);
        saveVoltammogram(j, i_forward - i_backward);
        if (debugLevel) {
            Serial.println("EOL");
        }
    }
}

static void runSWVBackward(int16_t startV, int16_t endV, int16_t pulseAmp, int16_t stepV, uint16_t delay_ms) {
    float i_forward = 0;
    float i_backward = 0;
    for (int16_t j = startV; j >= endV; j -= stepV) {
        i_forward = biasAndSample(j + pulseAmp, delay_ms);
        i_backward = biasAndSample(j - pulseAmp, delay_ms);
        saveVoltammogram(j, i_forward - i_backward);
        if (debugLevel) {
            Serial.println("EOL");
        }
    }
}

//────────────────────────────────────────────────────────────
// Public SWV Function
//────────────────────────────────────────────────────────────

void runSWV(uint8_t newGain, int16_t startV, int16_t endV,
            int16_t pulseAmp, int16_t stepV, double freq, bool setToZero) {
    LMPgainGLOBAL = newGain;
    stepV = abs(stepV);
    pulseAmp = abs(pulseAmp);

    // Convert frequency (Hz) to delay (ms) for half period.
    uint16_t delay_ms = (uint16_t)(1000.0 / (2 * freq));
    if (delay_ms > 1)
        delay_ms -= 1;

    reset_Voltammogram_arrays();
    arr_cur_index = 0;
    number_of_valid_points_in_volts_amps_array = 0;

    pstatInit(newGain);

    if (startV < endV)
        runSWVForward(startV, endV, pulseAmp, stepV, delay_ms);
    else
        runSWVBackward(startV, endV, pulseAmp, stepV, delay_ms);

    arr_cur_index = 0;
    if (setToZero)
        setOutputsToZero();

    if (debugLevel) {
        Serial.println("runSWV complete.");
    }
}

//────────────────────────────────────────────────────────────
// Data Logging Functions (Public)
//────────────────────────────────────────────────────────────

void writeVoltammogramToFile() {
    File file = SPIFFS.open("/data.csv", FILE_WRITE);
    if (!file) {
        if (debugLevel) {
            Serial.println("Error opening /data.csv for writing");
        }
        return;
    }
    file.println("Index,Current_Amps,Voltage_V,Time_ms");
    for (uint16_t i = 0; i < number_of_valid_points_in_volts_amps_array; i++) {
        file.print(i);
        file.print(",");
        file.print(amps[i], DEC);
        file.print(",");
        file.print((float)volts[i] / 1000.0, 3);
        file.print(",");
        file.println(time_Voltammaogram[i] - time_Voltammaogram[0]);
    }
    file.close();
    if (debugLevel) {
        Serial.println("Voltammogram data saved to /data.csv");
    }
}

void readFileAndSendOverSerial() {
    File file = SPIFFS.open("/data.csv", FILE_READ);
    if (!file) {
        if (debugLevel) {
            Serial.println("Error opening /data.csv for reading");
        }
        return;
    }
    if (debugLevel) {
        Serial.println("Reading /data.csv:");
    }
    while (file.available()) {
        Serial.write(file.read());
    }
    file.close();
}

void clearVoltammogramFile() {
    File file = SPIFFS.open("/data.csv", FILE_WRITE);
    if (!file) {
        if (debugLevel) {
            Serial.println("Error clearing /data.csv");
        }
        return;
    }
    file.close();
    if (debugLevel) {
        Serial.println("/data.csv cleared");
    }
}

void clearVoltammogramArrays() {
    reset_Voltammogram_arrays();
    arr_cur_index = 0;
    number_of_valid_points_in_volts_amps_array = 0;
    if (debugLevel) {
        Serial.println("Voltammogram arrays cleared");
    }
}

//────────────────────────────────────────────────────────────
// Debug Control Functions (Public)
//────────────────────────────────────────────────────────────

void setPstatDebug(bool on) {
    debugLevel = on;
    if (debugLevel) {
        Serial.println("pstat debug enabled.");
    }
}

bool pstatDebugEnabled() {
    return debugLevel;
}

//────────────────────────────────────────────────────────────
// Public Helper Functions for pstat Settings
//────────────────────────────────────────────────────────────

void updatePstatGain(uint8_t newGain) {
    LMPgainGLOBAL = newGain;
    pstatInit(newGain);
    if (debugLevel) {
        Serial.print("Pstat gain updated to: ");
        Serial.println(newGain);
    }
}

uint8_t getPstatGain() {
    return LMPgainGLOBAL;
}

void updatePstatBias(uint8_t newBias) {
    bias_setting = newBias;
    pStat.setBias(newBias);
    if (debugLevel) {
        Serial.print("Pstat bias updated to: ");
        Serial.println(newBias);
    }
}

uint8_t getPstatBias() {
    return bias_setting;
}

void updateAdcAverages(int newAverage) {
    num_adc_readings_to_average = newAverage;
    if (debugLevel) {
        Serial.print("ADC averaging updated to: ");
        Serial.println(newAverage);
    }
}

int getAdcAverages() {
    return num_adc_readings_to_average;
}

//────────────────────────────────────────────────────────────
// Public pstat Initialization Function
//────────────────────────────────────────────────────────────

// Renamed from initLMP to pstatInit to avoid conflict with the library.
void pstatInit(uint8_t newGain) {
    pStat.disableFET();
    pStat.setGain(newGain);
    pStat.setRLoad(0);
    pStat.setExtRefSource();
    pStat.setIntZ(1);
    pStat.setThreeLead();
    pStat.setBias(0);
    pStat.setPosBias();
    setOutputsToZero();
    LMPgainGLOBAL = newGain;
    if (debugLevel) {
        Serial.print("pstatInit: pstat initialized with gain ");
        Serial.println(newGain);
    }
}
