#include <LoRa.h>
#include <SPI.h>

#define ss 5
#define rst 14
#define dio0 2

#define START_MARKER 0x7E

uint8_t calculate_checksum(uint8_t *data, uint8_t length) {
  uint8_t checksum = 0;
  for (uint8_t i = 0; i < length; i++) {
    checksum ^= data[i];  // XOR all payload bytes
  }
  return checksum;
}

void process_message(uint8_t sensor_type, uint8_t sensor_id, uint8_t *payload, uint8_t length) {
  Serial.print("Received sensor type: ");
  Serial.println((char)sensor_type);  // Sensor type

  Serial.print("Sensor ID: ");
  Serial.println(sensor_id);  // Sensor ID

  Serial.print("Payload: ");
  for (uint8_t i = 0; i < length; i++) {
    Serial.print((char)payload[i]);  // Payload as string
  }
  Serial.println();
}

void setup() {
  Serial.begin(115200);
  while (!Serial);
  Serial.println("LoRa Receiver with CHYappy Protocol");

  LoRa.setPins(ss, rst, dio0);  // Setup LoRa transceiver module

  while (!LoRa.begin(433E6)) {  // Frequency setup
    Serial.println(".");
    delay(500);
  }

  LoRa.setSyncWord(0xA5);  // Sync word to match sender and receiver
  LoRa.setTxPower(20);        // Max power for long range
  LoRa.setSpreadingFactor(12);  // Increase SF for long range
  LoRa.setSignalBandwidth(125E3);  // Narrow bandwidth for better range
  LoRa.setCodingRate4(8);     // Max coding rate for robustness
  
  Serial.println("LoRa Initializing OK!");
}

void loop() {
  int packetSize = LoRa.parsePacket();  // Try to parse packet
  if (packetSize > 5) {  // Minimum valid packet size (start marker + length + sensor type + sensor ID + checksum)
    if (LoRa.read() == START_MARKER) {  // Start marker validation
      uint8_t length = LoRa.read();     // Read payload length
      uint8_t sensor_type = LoRa.read();  // Read sensor type
      uint8_t sensor_id = LoRa.read();  // Read sensor ID

      uint8_t payload[length];
      for (uint8_t i = 0; i < length; i++) {
        payload[i] = LoRa.read();  // Read payload
      }

      uint8_t received_checksum = LoRa.read();  // Read checksum

      // Validate checksum
      if (calculate_checksum(payload, length) == received_checksum) {
        process_message(sensor_type, sensor_id, payload, length);  // Process valid message
      } else {
        Serial.println("Checksum error");
      }

      Serial.print("Received packet with RSSI: ");
      Serial.println(LoRa.packetRssi());  // Print RSSI
    }
  }
}
