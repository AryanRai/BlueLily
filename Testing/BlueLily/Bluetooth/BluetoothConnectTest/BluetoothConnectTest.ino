#define HC05_TX 1  // Teensy TX1 (HC-05 RX)
#define HC05_RX 0  // Teensy RX1 (HC-05 TX)

void setup() {
    Serial.begin(115200);  // USB Serial for debugging
    Serial1.begin(9600);   // Bluetooth module (default HC-05 baud rate)

    Serial.println("HC-05 Bluetooth Module Ready");
}

void loop() {
    if (Serial1.available()) {
        Serial.write(Serial1.read());  // Forward data from HC-05 to Serial Monitor
    }
    if (Serial.available()) {
        Serial1.write(Serial.read());  // Forward data from Serial Monitor to HC-05
    }
}
