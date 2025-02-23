#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>
#include "Config.h"

void initSensors();

#if ENABLE_MAX31855
float readTemperature(); // Returns temperature in Celsius
#else
inline float readTemperature() { return -1.0; } // Default error value
#endif

#if ENABLE_MPU6500
void readIMU(float &accelX, float &accelY, float &accelZ, float &gyroX, float &gyroY, float &gyroZ);
#else
inline void readIMU(float &accelX, float &accelY, float &accelZ, float &gyroX, float &gyroY, float &gyroZ) {
  accelX = accelY = accelZ = gyroX = gyroY = gyroZ = 0.0;
}
#endif

#if ENABLE_ADS1115
int16_t readADC(uint8_t channel); // Raw ADC value
float readADCVoltage(uint8_t channel); // Voltage
#else
inline int16_t readADC(uint8_t) { return 0; }
inline float readADCVoltage(uint8_t) { return 0.0; }
#endif

#endif