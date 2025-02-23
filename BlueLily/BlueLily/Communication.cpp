#include <SPI.h>
#include "Communication.h"
#include "Configurator.h" // For passing received data

#if ENABLE_LORA
#include <LoRa.h>
LoRaClass LoRaModule;
#endif

#if ENABLE_CANBUS
#include <mcp_canbus.h>
MCP_CAN CAN(CAN_CS_PIN);
#endif

#if ENABLE_RS485
#define rs485 Serial2
#endif

// chYAPpy v1.2 CRC-8
#if ENABLE_RS485 || ENABLE_LORA
uint8_t crc8(const uint8_t* data, uint8_t len) {
  uint8_t crc = 0;
  for (uint8_t i = 0; i < len; i++) {
    crc ^= data[i];
    for (uint8_t j = 0; j < 8; j++) {
      if (crc & 0x80) crc = (crc << 1) ^ 0x31;
      else crc <<= 1;
    }
  }
  return crc;
}

void sendChYAPpyV12(Stream& output, uint8_t sensorType, uint8_t sensorId, uint16_t seqNum, uint8_t payloadType, const uint8_t* payload, uint8_t length, bool useTxEnable = false) {
  uint8_t message[length + 8];
  message[0] = CHYAPPY_V1_2_START;
  message[1] = length;
  message[2] = sensorType;
  message[3] = sensorId;
  message[4] = seqNum >> 8;
  message[5] = seqNum & 0xFF;
  message[6] = payloadType;
  for (uint8_t i = 0; i < length; i++) {
    message[7 + i] = payload[i];
  }
  message[length + 7] = crc8(&message[1], length + 6);

  if (useTxEnable) digitalWrite(RS485_TX_EN_PIN, HIGH);
  delayMicroseconds(10);
  output.write(message, length + 8);
  output.flush();
  if (useTxEnable) {
    delayMicroseconds(10);
    digitalWrite(RS485_TX_EN_PIN, LOW);
  }
}
#endif

void initCommunication() {
  Serial.begin(115200);

#if ENABLE_RS485
  pinMode(RS485_TX_EN_PIN, OUTPUT);
  digitalWrite(RS485_TX_EN_PIN, LOW);
  rs485.begin(RS485_BAUD);
  Serial.println("RS485 Initialized");
#endif

#if ENABLE_CANBUS
  while (CAN_OK != CAN.begin(CANBUS_BAUD)) {
    Serial.println("CAN BUS FAIL!");
    delay(100);
  }
  Serial.println("CAN BUS OK!");
#endif

#if ENABLE_BLUETOOTH
  Serial1.begin(BLUETOOTH_BAUD);
  Serial.println("Bluetooth Initialized");
#endif

#if ENABLE_LORA
  LoRaModule.setPins(LORA_SS_PIN, LORA_RST_PIN, LORA_DIO0_PIN);
  while (!LoRaModule.begin(LORA_FREQ)) {
    Serial.println("LoRa Init Failed. Retrying...");
    delay(500);
  }
  LoRaModule.setSyncWord(0xA5);
  LoRaModule.setTxPower(20);
  LoRaModule.setSpreadingFactor(12);
  LoRaModule.setSignalBandwidth(125E3);
  LoRaModule.setCodingRate4(8);
  Serial.println("LoRa Initialized");
#endif
}

#if ENABLE_RS485
void sendRS485(uint8_t sensorType, uint8_t sensorId, uint16_t seqNum, const char* data, uint8_t payloadType) {
  uint8_t length = (payloadType == PAYLOAD_TYPE_STRING) ? strlen(data) : (payloadType == PAYLOAD_TYPE_FLOAT ? 4 : 0);
  sendChYAPpyV12(rs485, sensorType, sensorId, seqNum, payloadType, (const uint8_t*)data, length, true);
}

void receiveRS485() {
  static uint8_t buffer[CONFIG_BUFFER_SIZE];
  static uint8_t pos = 0;
  while (rs485.available() && pos < CONFIG_BUFFER_SIZE) {
    buffer[pos++] = rs485.read();
    if (pos >= 8 && buffer[0] == CHYAPPY_V1_2_START) { // Minimum chYAPpy v1.2 size
      uint8_t length = buffer[1];
      if (pos >= length + 8) { // Full message received
        uint8_t crc = crc8(&buffer[1], length + 6);
        if (crc == buffer[length + 7]) {
          uint8_t sensorType = buffer[2];
          uint8_t sensorId = buffer[3];
          uint16_t seqNum = (buffer[4] << 8) | buffer[5];
          uint8_t payloadType = buffer[6];
          char payload[CONFIG_BUFFER_SIZE];
          memcpy(payload, &buffer[7], length);
          payload[length] = '\0';

          Serial.print("RS485 Parsed - Type: ");
          Serial.print((char)sensorType);
          Serial.print(", ID: ");
          Serial.print(sensorId);
          Serial.print(", Seq: ");
          Serial.print(seqNum);
          Serial.print(", Payload: ");
          if (payloadType == PAYLOAD_TYPE_STRING) Serial.println(payload);
          else if (payloadType == PAYLOAD_TYPE_FLOAT) Serial.println(*(float*)payload);

          handleConfigCommand(METHOD_RS485, sensorType, sensorId, seqNum, payloadType, payload);
          pos = 0; // Reset buffer
        } else {
          Serial.println("RS485 CRC Error");
          pos = 0; // Reset on error
        }
      }
    }
  }
}
#endif

#if ENABLE_CANBUS
void sendCANBUS(uint32_t id, const uint8_t* data, uint8_t len) {
  CAN.sendMsgBuf(id, 0, len, (unsigned char*)data);
}

void receiveCANBUS() {
  unsigned char len = 0;
  unsigned char buf[8];
  unsigned long canId;
  if (CAN_MSGAVAIL == CAN.checkReceive()) {
    CAN.readMsgBuf(&len, buf);
    canId = CAN.getCanId();
    Serial.print("CAN Received - ID: 0x");
    Serial.print(canId, HEX);
    Serial.print(", Data: ");
    for (int i = 0; i < len; i++) {
      Serial.print(buf[i], HEX);
      Serial.print(" ");
    }
    Serial.println();
    char payload[CONFIG_BUFFER_SIZE];
    memcpy(payload, buf, len);
    payload[len] = '\0';
    handleConfigCommand(METHOD_CANBUS, 0, 0, 0, PAYLOAD_TYPE_STRING, payload); // No chYAPpy for CAN
  }
}
#endif

#if ENABLE_BLUETOOTH
void sendBluetooth(const char* data) {
  Serial1.print(data);
}

void receiveBluetooth() {
  static char buffer[CONFIG_BUFFER_SIZE];
  static uint8_t pos = 0;
  while (Serial1.available() && pos < CONFIG_BUFFER_SIZE - 1) {
    char c = Serial1.read();
    if (c == '\n' || c == '\r') {
      buffer[pos] = '\0';
      Serial.print("Bluetooth Received: ");
      Serial.println(buffer);
      handleConfigCommand(METHOD_BLUETOOTH, 0, 0, 0, PAYLOAD_TYPE_STRING, buffer);
      pos = 0;
    } else {
      buffer[pos++] = c;
    }
  }
}
#endif

#if ENABLE_LORA
void sendLoRa(uint8_t sensorType, uint8_t sensorId, uint16_t seqNum, const char* data, uint8_t payloadType) {
  LoRaModule.beginPacket();
  uint8_t length = (payloadType == PAYLOAD_TYPE_STRING) ? strlen(data) : (payloadType == PAYLOAD_TYPE_FLOAT ? 4 : 0);
  sendChYAPpyV12(LoRaModule, sensorType, sensorId, seqNum, payloadType, (const uint8_t*)data, length, false);
  LoRaModule.endPacket();
}

void receiveLoRa() {
  int packetSize = LoRaModule.parsePacket();
  if (packetSize) {
    static uint8_t buffer[CONFIG_BUFFER_SIZE];
    uint8_t pos = 0;
    while (LoRaModule.available() && pos < CONFIG_BUFFER_SIZE) {
      buffer[pos++] = LoRaModule.read();
    }
    if (pos >= 8 && buffer[0] == CHYAPPY_V1_2_START) {
      uint8_t length = buffer[1];
      if (pos >= length + 8) {
        uint8_t crc = crc8(&buffer[1], length + 6);
        if (crc == buffer[length + 7]) {
          uint8_t sensorType = buffer[2];
          uint8_t sensorId = buffer[3];
          uint16_t seqNum = (buffer[4] << 8) | buffer[5];
          uint8_t payloadType = buffer[6];
          char payload[CONFIG_BUFFER_SIZE];
          memcpy(payload, &buffer[7], length);
          payload[length] = '\0';

          Serial.print("LoRa Parsed - Type: ");
          Serial.print((char)sensorType);
          Serial.print(", ID: ");
          Serial.print(sensorId);
          Serial.print(", Seq: ");
          Serial.print(seqNum);
          Serial.print(", Payload: ");
          if (payloadType == PAYLOAD_TYPE_STRING) Serial.println(payload);
          else if (payloadType == PAYLOAD_TYPE_FLOAT) Serial.println(*(float*)payload);

          handleConfigCommand(METHOD_LORA, sensorType, sensorId, seqNum, payloadType, payload);
        } else {
          Serial.println("LoRa CRC Error");
        }
      }
    }
  }
}
#endif