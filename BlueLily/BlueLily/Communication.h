#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <Arduino.h>
#include "Config.h"

void initCommunication();

#if ENABLE_RS485
void sendRS485(uint8_t sensorType, uint8_t sensorId, uint16_t seqNum, const char* data, uint8_t payloadType = PAYLOAD_TYPE_STRING);
void receiveRS485();
#else
inline void sendRS485(uint8_t, uint8_t, uint16_t, const char*, uint8_t = PAYLOAD_TYPE_STRING) {}
inline void receiveRS485() {}
#endif

#if ENABLE_CANBUS
void sendCANBUS(uint32_t id, const uint8_t* data, uint8_t len);
void receiveCANBUS();
#else
inline void sendCANBUS(uint32_t, const uint8_t*, uint8_t) {}
inline void receiveCANBUS() {}
#endif

#if ENABLE_BLUETOOTH
void sendBluetooth(const char* data);
void receiveBluetooth();
#else
inline void sendBluetooth(const char*) {}
inline void receiveBluetooth() {}
#endif

#if ENABLE_LORA
void sendLoRa(uint8_t sensorType, uint8_t sensorId, uint16_t seqNum, const char* data, uint8_t payloadType = PAYLOAD_TYPE_STRING);
void receiveLoRa();
#else
inline void sendLoRa(uint8_t, uint8_t, uint16_t, const char*, uint8_t = PAYLOAD_TYPE_STRING) {}
inline void receiveLoRa() {}
#endif

#endif