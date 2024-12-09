#include <LoRa.h>
#include <SPI.h>
#include <string.h>

#define ss 5
#define rst 14
#define dio0 2

#define START_MARKER 0x7E

int counter = 0;

void send_message(uint8_t sensor_type, uint8_t sensor_id, uint8_t *payload, uint8_t length) {
  uint8_t message[length + 5];
  uint8_t checksum = 0;

  message[0] = START_MARKER;  // Start marker
  message[1] = length;        // Payload length
  message[2] = sensor_type;   // Sensor type
  message[3] = sensor_id;     // Sensor ID

  // Copy payload and calculate checksum
  for (uint8_t i = 0; i < length; i++) {
    message[4 + i] = payload[i];
    checksum ^= payload[i];  // XOR for checksum
  }

  message[length + 4] = checksum;  // Add checksum

  LoRa.beginPacket();
  LoRa.write(message, length + 5);  // Send entire message
  LoRa.endPacket();
}

void setup() {
  Serial.begin(115200);
  while (!Serial);
  Serial.println("LoRa Sender with CHYappy Protocol");

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
  Serial.print("Sending packet: ");
  Serial.println(counter);

  uint8_t sensor_type = 'T';  // Example sensor type 'T' (temperature)
  uint8_t sensor_id = 1;      // Sensor ID
  float sensor_value = counter;  // Example sensor value
  char payload[10];

  snprintf(payload, sizeof(payload), "%.4f", sensor_value);  // Convert float to string
  uint8_t length = strlen(payload);

  send_message(sensor_type, sensor_id, (uint8_t *)payload, length);

  counter++;

  delay(10);
}
