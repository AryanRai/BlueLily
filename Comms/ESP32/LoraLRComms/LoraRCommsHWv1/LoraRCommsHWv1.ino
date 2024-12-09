#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <LoRa.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

// Define custom I2C pins
#define CUSTOM_SDA_PIN 26  // Custom SDA pin
#define CUSTOM_SCL_PIN 27  // Custom SCL pin

// Declaration for an SSD1306 display connected to I2C
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// LoRa pins and settings
#define ss 5
#define rst 14
#define dio0 2
#define START_MARKER 0x7E

// Variables for non-blocking display
unsigned long lastUpdateTime = 0;
const unsigned long updateInterval = 1000; // Time between display updates
String currentSensorInfo = "";
String currentPayloadInfo = "";
bool displayData = false;

void displayMessage(String sensorInfo, String payloadInfo);
void printReceivedMessageRaw(uint8_t *message, uint8_t length);

uint8_t calculate_checksum(uint8_t *data, uint8_t length) {
  uint8_t checksum = 0;
  for (uint8_t i = 0; i < length; i++) {
    checksum ^= data[i];  // XOR all payload bytes
  }
  return checksum;
}

void process_message(uint8_t sensor_type, uint8_t sensor_id, uint8_t *payload, uint8_t length) {
  // Prepare sensor info and payload for display
  currentSensorInfo = "Type: " + String((char)sensor_type) + " ID: " + String(sensor_id);
  currentPayloadInfo = "";

  for (uint8_t i = 0; i < length; i++) {
    currentPayloadInfo += (char)payload[i];  // Append payload as string
  }

  printReceivedMessage(sensor_type, sensor_id, payload, length);

  // Create a message buffer to send raw data to the Serial Monitor
  uint8_t rawMessage[2 + length + 3]; // Start marker, length, sensor type, sensor ID, payload, checksum
  rawMessage[0] = START_MARKER; // Start marker
  rawMessage[1] = length; // Length of the payload
  rawMessage[2] = sensor_type; // Sensor type
  rawMessage[3] = sensor_id; // Sensor ID

  // Copy payload into the raw message
  for (uint8_t i = 0; i < length; i++) {
    rawMessage[4 + i] = payload[i];
  }
  
  // Calculate checksum and append
  rawMessage[4 + length] = calculate_checksum(payload, length);

  // Print the raw message to Serial Monitor
  printReceivedMessageRaw(rawMessage, sizeof(rawMessage));

  // Set flag to update display
  displayData = true; 
}

void printReceivedMessage(uint8_t sensor_type, uint8_t sensor_id, uint8_t *payload, uint8_t length) {
  Serial.print("Received sensor type: ");
  Serial.println((char)sensor_type);  // Print sensor type

  Serial.print("Sensor ID: ");
  Serial.println(sensor_id);  // Print sensor ID

  Serial.print("Payload: ");
  for (uint8_t i = 0; i < length; i++) {
    Serial.print((char)payload[i]);  // Print payload as string
  }
  Serial.println();
}

void printReceivedMessageRaw(uint8_t *message, uint8_t length) {
  Serial.println("Received raw message: ");
  for (uint8_t i = 0; i < length; i++) {
    Serial.print(message[i], HEX); // Print each byte in hexadecimal format
    Serial.print(":");
  }
  Serial.println();
}

void setup() {
  // Initialize serial and LoRa communication
  Serial.begin(115200);
  while (!Serial);
  Serial.println("LoRa Receiver with CHYappy Protocol");

  // Setup LoRa transceiver module
  LoRa.setPins(ss, rst, dio0);  
  while (!LoRa.begin(433E6)) {
    Serial.println(".");
    delay(500);
  }

  LoRa.setSyncWord(0xA5);
  LoRa.setTxPower(20);        
  LoRa.setSpreadingFactor(12);
  LoRa.setSignalBandwidth(125E3);
  LoRa.setCodingRate4(8);     

  Serial.println("LoRa Initializing OK!");

  // Initialize OLED display
  Wire.begin(CUSTOM_SDA_PIN, CUSTOM_SCL_PIN);
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("LoRa Initialized!");
  display.display();
  delay(2000);
}

void loop() {
  // Receive and process LoRa packets
  int packetSize = LoRa.parsePacket();
  if (packetSize > 5) {
    if (LoRa.read() == START_MARKER) {
      uint8_t length = LoRa.read();
      uint8_t sensor_type = LoRa.read();
      uint8_t sensor_id = LoRa.read();
      uint8_t payload[length];

      for (uint8_t i = 0; i < length; i++) {
        payload[i] = LoRa.read();
      }

      uint8_t received_checksum = LoRa.read();

      if (calculate_checksum(payload, length) == received_checksum) {
        process_message(sensor_type, sensor_id, payload, length);
      } else {
        Serial.println("Checksum error");
      }

      Serial.print("RSSI: ");
      Serial.println(LoRa.packetRssi());
    }
  }

  // Non-blocking display update
  if (displayData) {
    unsigned long currentTime = millis();
    if (currentTime - lastUpdateTime >= updateInterval) {
      lastUpdateTime = currentTime;
      displayMessage(currentSensorInfo, currentPayloadInfo);
      displayData = false; // Reset the flag after displaying
    }
  }
}

void displayMessage(String sensorInfo, String payloadInfo) {
  display.clearDisplay();
  
  // Display sensor information
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println(sensorInfo);

  // Display payload information with 2x text size for emphasis
  display.setTextSize(2);
  display.setCursor(0, 16);
  display.println(payloadInfo);

  display.display();

  // Optional: Add a little animation or invert effect
  //display.invertDisplay(true);
  //delay(300); // Brief delay for effect
  //display.invertDisplay(false);
}
