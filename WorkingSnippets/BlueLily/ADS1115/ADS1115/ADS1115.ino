#include <Wire.h>
#include <Adafruit_ADS1X15.h>

// Create an ADS1115 object
Adafruit_ADS1115 ads;

void setup() {
  Serial.begin(115200);
  while (!Serial);  // Wait for Serial Monitor to open
  Serial.println("Initializing ADS1115...");

  // Initialize the ADS1115
  if (!ads.begin(0x48)) {  // Default I2C address (ADDR connected to GND)
    Serial.println("Failed to initialize ADS1115! Check wiring.");
    while (1);
  }
  Serial.println("ADS1115 Initialized.");
  
  // Set gain to ±4.096V (gain=1) for maximum range; adjust as needed
  ads.setGain(GAIN_ONE);
}

void loop() {
  // Read analog values from all four channels
  int16_t adc0 = ads.readADC_SingleEnded(0);  // Channel 0
  int16_t adc1 = ads.readADC_SingleEnded(1);  // Channel 1
  int16_t adc2 = ads.readADC_SingleEnded(2);  // Channel 2
  int16_t adc3 = ads.readADC_SingleEnded(3);  // Channel 3

  // Convert raw ADC values to voltage (assuming ±4.096V range)
  float voltage0 = adc0 * 0.125 / 1000;  // Conversion factor: 0.125 mV/LSB
  float voltage1 = adc1 * 0.125 / 1000;
  float voltage2 = adc2 * 0.125 / 1000;
  float voltage3 = adc3 * 0.125 / 1000;

  // Print readings to Serial Monitor
  Serial.print("ADC0: "); Serial.print(adc0); Serial.print(" ("); Serial.print(voltage0); Serial.println(" V)");
  Serial.print("ADC1: "); Serial.print(adc1); Serial.print(" ("); Serial.print(voltage1); Serial.println(" V)");
  Serial.print("ADC2: "); Serial.print(adc2); Serial.print(" ("); Serial.print(voltage2); Serial.println(" V)");
  Serial.print("ADC3: "); Serial.print(adc3); Serial.print(" ("); Serial.print(voltage3); Serial.println(" V)");
  Serial.println("--------------------------------");

  delay(1000);  // Wait 1 second between readings
}
