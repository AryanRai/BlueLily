#include <SPI.h>
#include <SD.h>
#include <Adafruit_SPIFlash.h>

#define USE_FLASH false


// Pin Definitions
#define FLASH_CS 10  // W25Q128 Chip Select
#define SD_CS BUILTIN_SDCARD  // Teensy 4.1 SD Card

// Flash and SD objects
Adafruit_FlashTransport_SPI flashTransport(FLASH_CS, &SPI);
Adafruit_SPIFlash flash(&flashTransport);
File sdFile;

// Dummy Sensor Data
float dummyTemperature = 25.6;
float dummyPressure = 1013.25;

// Flags to enable/disable Flash and SD Card
#define USE_FLASH true    // Set to false to skip Flash initialization and writing
#define USE_SDCARD true   // Set to false to skip SD card initialization and writing

void setup() {
  Serial.begin(115200);
  while (!Serial);

  // Initialize Flash Memory if enabled
  if (USE_FLASH) {
    if (!flash.begin()) {
      Serial.println("Failed to initialize W25Q128 flash memory!");
      while (1);
    }
    Serial.println("W25Q128 Flash initialized.");
  }

  // Initialize SD Card if enabled
  if (USE_SDCARD) {
    if (!SD.begin(SD_CS)) {
      Serial.println("Failed to initialize SD card!");
      while (1);
    }
    Serial.println("SD card initialized.");
  }
}

void loop() {
  // Write to Flash Memory if enabled
  if (USE_FLASH) {
    uint32_t address = 0;
    char flashData[50];
    snprintf(flashData, sizeof(flashData), "Temp: %.2f, Pressure: %.2f\n", dummyTemperature, dummyPressure);

    if (!flash.writeBuffer(address, (uint8_t *)flashData, strlen(flashData))) {
      Serial.println("Failed to write to flash memory!");
    } else {
      Serial.println("Data written to flash memory:");
      Serial.print(flashData);
    }
  }

  // Write to SD Card if enabled
  if (USE_SDCARD) {
    sdFile = SD.open("sensor_data.txt", FILE_WRITE);
    if (!sdFile) {
      Serial.println("Failed to open file on SD card!");
    } else {
      sdFile.printf("Temp: %.2f, Pressure: %.2f\n", dummyTemperature, dummyPressure);
      sdFile.close();
      Serial.println("Data written to SD card.");
    }
  }

  // Simulate changing sensor values
  dummyTemperature += 0.1;
  dummyPressure += 0.05;

  delay(1000);  // Wait 1 second
}
