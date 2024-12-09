// RS485 connections
#define TX_EN_PIN 7       // RE and DE pin connected to RS485 module for controlling transmission and reception
#define START_MARKER 0x7E // Start marker for the protocol

// Use Teensy Serial1 for RS485
#define rs485 Serial1

void setup() {
  pinMode(TX_EN_PIN, OUTPUT);       // Set TX_EN_PIN as output
  digitalWrite(TX_EN_PIN, LOW);     // Set to receive mode initially
  
  rs485.begin(115200);              // Set baud rate for RS485 communication
  
  Serial.begin(115200);             // Debugging via USB Serial
  while (!Serial);
  Serial.println("RS485 Communication Initialized");
}

void send_message(uint8_t sensor_type, uint8_t sensor_id, uint8_t *payload, uint8_t length) {
  uint8_t message[length + 5];
  uint8_t checksum = 0;

  message[0] = START_MARKER;        // Start marker
  message[1] = length;              // Payload length
  message[2] = sensor_type;         // Sensor type
  message[3] = sensor_id;           // Sensor ID

  // Copy payload and calculate checksum
  for (uint8_t i = 0; i < length; i++) {
    message[4 + i] = payload[i];
    checksum ^= payload[i];
  }

  message[length + 4] = checksum;   // Add checksum

  digitalWrite(TX_EN_PIN, HIGH);    // Enable RS485 Transmitter
  delayMicroseconds(10);            // Small delay to ensure DE/RE settles
  rs485.write(message, length + 5); // Transmit the message
  rs485.flush();                    // Wait until the transmission is complete
  delayMicroseconds(10);            // Small delay to ensure transmission is finished
  digitalWrite(TX_EN_PIN, LOW);     // Disable RS485 Transmitter (back to receive mode)
}

void loop() {
  // Example call: Thermocouple1 T1 value 25.3662
  uint8_t sensor_type = 'T';        // Sensor type: Temperature
  uint8_t sensor_id = 1;            // Sensor ID
  float sensor_value = 25.3662;     // Example sensor value
  char payload[10];                 // Buffer for the payload

  snprintf(payload, sizeof(payload), "%.4f", sensor_value); // Format float to string
  uint8_t length = strlen(payload); // Get payload length

  send_message(sensor_type, sensor_id, (uint8_t *)payload, length);

  Serial.println("Message sent over RS485!");
  delay(1000);                      // Delay 1 second between messages
}
