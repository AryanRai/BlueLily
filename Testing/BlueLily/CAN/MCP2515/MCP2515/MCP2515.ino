#include <AA_MCP2515.h>

AA_MCP2515 CAN;

void setup() {
  Serial.begin(115200);
  while (!Serial);

  // Initialize MCP2515
  if (CAN.begin(500000) == MCP2515::ERROR_OK) {
    Serial.println("CAN initialized successfully!");
  } else {
    Serial.println("CAN initialization failed! Check wiring and power.");
    while (1);
  }
}

void loop() {
  // Data to send
  uint8_t data[] = {0x01, 0x02, 0x03, 0x04};

  // Send CAN message with ID 0x100
  if (CAN.sendMessage(0x100, data, sizeof(data)) == MCP2515::ERROR_OK) {
    Serial.println("Message sent!");
  } else {
    Serial.println("Message sending failed!");
  }

  delay(1000);
}
