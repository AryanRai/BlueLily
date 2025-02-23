#include <SdFat.h>
#include "Logger.h"
#include "Config.h"

#if ENABLE_SD
#include "RingBuf.h"
SdFs sd;
FsFile file;
RingBuf<FsFile, RING_BUF_CAPACITY> rb;
#define LOG_FILENAME "FlightLog.csv"
#endif

#if ENABLE_W25Q128
#include <SPIFlash.h>
SPIFlash flash(W25Q128_CS_PIN); // Constructor initializes with CS pin
uint32_t flashAddress = 0;      // Global variable for flash write position
#endif

void initLogger() {
#if ENABLE_SD
  if (!sd.begin(SD_CONFIG)) {
    Serial.println("SD init failed!");
    return;
  }
  if (!file.open(LOG_FILENAME, O_RDWR | O_CREAT | O_TRUNC)) {
    Serial.println("SD file open failed!");
    return;
  }
  if (!file.preAllocate(SD_LOG_FILE_SIZE)) {
    Serial.println("SD preAllocate failed!");
    file.close();
    return;
  }
  rb.begin(&file);
  Serial.println("SD Logger Initialized");
#endif

#if ENABLE_W25Q128
  if (!flash.initialize()) { // Check if flash chip is present
    Serial.println("W25Q128 init failed! Check wiring or chip ID.");
    return;
  }
  flashAddress = 0; // Reset position
  Serial.println("W25Q128 Logger Initialized");
#endif
}

void logData(const char* data) {
#if ENABLE_SD
  if (rb.bytesUsed() + file.curPosition() < SD_LOG_FILE_SIZE - 20) {
    rb.println(data);
    if (rb.getWriteError()) {
      Serial.println("SD RingBuf write error");
    }
    if (rb.bytesUsed() >= 512 && !file.isBusy()) {
      if (512 != rb.writeOut(512)) {
        Serial.println("SD writeOut failed");
      }
    }
  } else {
    Serial.println("SD file full");
  }
#endif

#if ENABLE_W25Q128
  if (flashAddress < W25Q128_CAPACITY - strlen(data) - 2) {
    flash.writeBytes(flashAddress, (uint8_t*)data, strlen(data));
    flashAddress += strlen(data);
    flash.writeByte(flashAddress++, '\r');
    flash.writeByte(flashAddress++, '\n');
  } else {
    Serial.println("W25Q128 full");
  }
#endif
}

void flushLogger() {
#if ENABLE_SD
  rb.sync();
  file.flush();
#endif
#if ENABLE_W25Q128
  // Flash writes are immediate
#endif
}

void closeLogger() {
#if ENABLE_SD
  flushLogger();
  file.truncate();
  file.close();
  Serial.println("SD Logger Closed");
#endif
#if ENABLE_W25Q128
  Serial.println("W25Q128 Logger Closed");
#endif
}

void previewLoggedData() {
#if ENABLE_SD
  if (!file.open(LOG_FILENAME, O_READ)) {
    Serial.println("SD file reopen failed for preview!");
    return;
  }
  Serial.println("SD Data Preview (first 20 lines):");
  uint8_t lineCount = 0;
  while (file.available() && lineCount < 20) {
    int c = file.read();
    if (c < 0) break;
    Serial.write(c);
    if (c == '\n') lineCount++;
  }
  Serial.println("--- End SD Preview ---");
  file.close();
#endif

#if ENABLE_W25Q128
  Serial.println("W25Q128 Data Preview (first 256 bytes or until end):");
  uint8_t buffer[256];
  uint32_t bytesToRead = min(256, flashAddress);
  flash.readBytes(0, buffer, bytesToRead);
  for (uint32_t i = 0; i < bytesToRead; i++) {
    Serial.write(buffer[i]);
  }
  Serial.println("--- End W25Q128 Preview ---");
#endif

#if !ENABLE_SD && !ENABLE_W25Q128
  Serial.println("No loggers enabled for preview");
#endif
}

void syncFlashToSD() {
#if ENABLE_SD && ENABLE_W25Q128
  if (!file.open(LOG_FILENAME, O_RDWR | O_APPEND)) {
    Serial.println("SD file reopen failed for sync!");
    return;
  }
  Serial.println("Syncing W25Q128 to SD...");
  uint8_t buffer[512];
  uint32_t bytesRead = 0;
  uint32_t totalBytes = flashAddress;

  while (bytesRead < totalBytes) {
    uint32_t chunkSize = min(512, totalBytes - bytesRead);
    flash.readBytes(bytesRead, buffer, chunkSize);
    if (rb.bytesUsed() + file.curPosition() + chunkSize < SD_LOG_FILE_SIZE) {
      rb.write(buffer, chunkSize);
      if (rb.getWriteError()) {
        Serial.println("SD RingBuf write error during sync");
        break;
      }
      if (rb.bytesUsed() >= 512 && !file.isBusy()) {
        if (512 != rb.writeOut(512)) {
          Serial.println("SD writeOut failed during sync");
          break;
        }
      }
    } else {
      Serial.println("SD file full during sync");
      break;
    }
    bytesRead += chunkSize;
  }
  rb.sync();
  file.flush();
  file.close();
  Serial.println("Sync complete");
#else
  Serial.println("Sync requires both SD and W25Q128 enabled");
#endif
}