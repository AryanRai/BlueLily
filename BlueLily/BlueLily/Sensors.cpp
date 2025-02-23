#include <Wire.h>
#include <SPI.h>
#include "Sensors.h"
#include "Config.h"

#if ENABLE_MAX31855
#include <Adafruit_MAX31855.h>
Adafruit_MAX31855 thermocouple(MAX31855_CS_PIN);
#endif

#if ENABLE_MPU6500
#include <FastIMU.h>
MPU6500 IMU;
calData calib = {0};
AccelData accelData;
GyroData gyroData;
#endif

#if ENABLE_ADS1115
#include <Adafruit_ADS1X15.h>
Adafruit_ADS1115 ads;
#endif

void initSensors() {
  Wire.begin();
  SPI.begin();

#if ENABLE_MAX31855
  if (!thermocouple.begin()) {
    Serial.println("ERROR: MAX31855 not detected!");
    while (1);
  }
  Serial.println("MAX31855 Initialized");
#endif

#if ENABLE_MPU6500
  Wire.setClock(400000);
  int err = IMU.init(calib, MPU6500_I2C_ADDR);
  if (err != 0) {
    Serial.print("Error initializing MPU6500: ");
    Serial.println(err);
    while (1);
  }
#ifdef PERFORM_CALIBRATION
  Serial.println("Calibrating MPU6500...");
  delay(5000);
  Serial.println("Keep IMU level.");
  IMU.calibrateAccelGyro(&calib);
  Serial.println("Calibration done!");
  Serial.print("Accel biases X/Y/Z: ");
  Serial.print(calib.accelBias[0]); Serial.print(", ");
  Serial.print(calib.accelBias[1]); Serial.print(", ");
  Serial.println(calib.accelBias[2]);
  Serial.print("Gyro biases X/Y/Z: ");
  Serial.print(calib.gyroBias[0]); Serial.print(", ");
  Serial.print(calib.gyroBias[1]); Serial.print(", ");
  Serial.println(calib.gyroBias[2]);
  delay(5000);
  IMU.init(calib, MPU6500_I2C_ADDR);
#endif
  Serial.println("MPU6500 Initialized");
#endif

#if ENABLE_ADS1115
  if (!ads.begin(ADS1115_I2C_ADDR)) {
    Serial.println("Failed to initialize ADS1115! Check wiring.");
    while (1);
  }
  ads.setGain(GAIN_ONE);
  Serial.println("ADS1115 Initialized");
#endif
}

#if ENABLE_MAX31855
float readTemperature() {
  float tempC = thermocouple.readCelsius();
  if (isnan(tempC)) {
    Serial.println("Error reading thermocouple!");
    return -1.0;
  }
  return tempC;
}
#endif

#if ENABLE_MPU6500
void readIMU(float &accelX, float &accelY, float &accelZ, float &gyroX, float &gyroY, float &gyroZ) {
  IMU.update();
  IMU.getAccel(&accelData);
  IMU.getGyro(&gyroData);
  accelX = accelData.accelX;
  accelY = accelData.accelY;
  accelZ = accelData.accelZ;
  gyroX = gyroData.gyroX;
  gyroY = gyroData.gyroY;
  gyroZ = gyroData.gyroZ;
}
#endif

#if ENABLE_ADS1115
int16_t readADC(uint8_t channel) {
  if (channel > 3) return 0;
  return ads.readADC_SingleEnded(channel);
}

float readADCVoltage(uint8_t channel) {
  int16_t adcValue = readADC(channel);
  return adcValue * 0.125 / 1000; // ±4.096V range
}
#endif