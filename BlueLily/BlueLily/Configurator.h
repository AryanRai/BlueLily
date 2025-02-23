#ifndef CONFIGURATOR_H
#define CONFIGURATOR_H

#include <Arduino.h>
#include "Config.h"
#include "Communication.h"

void initConfigurator();
void processCommunication();
void sendResponse(uint8_t method, uint8_t sensorId, uint16_t seqNum, const char* response);

void handleConfigCommand(uint8_t method, uint8_t sensorType, uint8_t sensorId, uint16_t seqNum, uint8_t payloadType, const char* payload);

enum CommMethod {
  METHOD_RS485 = 0,
  METHOD_CANBUS,
  METHOD_BLUETOOTH,
  METHOD_LORA
};

#endif