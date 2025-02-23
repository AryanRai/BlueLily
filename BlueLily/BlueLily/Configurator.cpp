#include "Configurator.h"

// Simple in-memory settings storage (key-value pairs)
struct Setting {
  char key[KEY_MAX_LEN];
  char value[VALUE_MAX_LEN];
};
static Setting settings[MAX_SETTINGS];
static uint8_t settingCount = 0;

void initConfigurator() {
  Serial.println("Configurator Initialized");
  settingCount = 0; // Reset settings
}

void processCommunication() {
#if ENABLE_RS485
  receiveRS485();
#endif
#if ENABLE_CANBUS
  receiveCANBUS();
#endif
#if ENABLE_BLUETOOTH
  receiveBluetooth();
#endif
#if ENABLE_LORA
  receiveLoRa();
#endif
}

void sendResponse(uint8_t method, uint8_t sensorId, uint16_t seqNum, const char* response) {
  switch (method) {
#if ENABLE_RS485
    case METHOD_RS485:
      sendRS485(SENSOR_TYPE_ACK, sensorId, seqNum, response);
      break;
#endif
#if ENABLE_CANBUS
    case METHOD_CANBUS:
      sendCANBUS(0x01, (const uint8_t*)response, strlen(response));
      break;
#endif
#if ENABLE_BLUETOOTH
    case METHOD_BLUETOOTH:
      sendBluetooth(response);
      break;
#endif
#if ENABLE_LORA
    case METHOD_LORA:
      sendLoRa(SENSOR_TYPE_ACK, sensorId, seqNum, response);
      break;
#endif
    default:
      Serial.println("Invalid response method");
  }
}

void handleConfigCommand(uint8_t method, uint8_t sensorType, uint8_t sensorId, uint16_t seqNum, uint8_t payloadType, const char* payload) {
  if (sensorType == SENSOR_TYPE_CONFIG && payloadType == PAYLOAD_TYPE_STRING) {
    // Parse "KEY=VALUE" format
    char key[KEY_MAX_LEN];
    char value[VALUE_MAX_LEN];
    if (sscanf(payload, "%15[^=]=%31s", key, value) == 2) {
      // Check if key exists, update or add
      for (uint8_t i = 0; i < settingCount; i++) {
        if (strcmp(settings[i].key, key) == 0) {
          strncpy(settings[i].value, value, VALUE_MAX_LEN);
          Serial.print("Updated setting: ");
          Serial.print(key);
          Serial.print("=");
          Serial.println(value);
          sendResponse(method, sensorId, seqNum, "ACK");
          return;
        }
      }
      if (settingCount < MAX_SETTINGS) {
        strncpy(settings[settingCount].key, key, KEY_MAX_LEN);
        strncpy(settings[settingCount].value, value, VALUE_MAX_LEN);
        settingCount++;
        Serial.print("Added setting: ");
        Serial.print(key);
        Serial.print("=");
        Serial.println(value);
        sendResponse(method, sensorId, seqNum, "ACK");
      } else {
        Serial.println("Settings full");
        sendResponse(method, sensorId, seqNum, "NACK - Settings full");
      }
    } else {
      Serial.println("Invalid config format");
      sendResponse(method, sensorId, seqNum, "NACK - Invalid format");
    }
  } else {
    // For non-config messages, just log
    Serial.println("Non-config message received");
  }
}