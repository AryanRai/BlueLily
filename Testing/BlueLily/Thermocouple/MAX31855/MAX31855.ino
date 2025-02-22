#include <SPI.h>
#include <Adafruit_MAX31855.h>

// Define MAX31855 SPI pins
#define MAX_CS 10  // Connect to CS pin of MAX31855

// Create MAX31855 object
Adafruit_MAX31855 thermocouple(MAX_CS);

void setup() {
    Serial.begin(115200);
    while (!Serial);  // Wait for Serial Monitor

    Serial.println("MAX31855 Thermocouple Test");

    if (!thermocouple.begin()) {
        Serial.println("ERROR: MAX31855 not detected!");
        while (1);
    }
}

void loop() {
    double tempC = thermocouple.readCelsius();
    double tempF = thermocouple.readFahrenheit();

    if (isnan(tempC)) {
        Serial.println("Error reading thermocouple!");
    } else {
        Serial.print("Temperature: ");
        Serial.print(tempC);
        Serial.println(" °C");

        Serial.print("Temperature: ");
        Serial.print(tempF);
        Serial.println(" °F");

        Serial.print("Internal Temp = ");
        Serial.println(thermocouple.readInternal());

        Serial.print("Error:" );
        Serial.println(thermocouple.readError());
    }

    delay(1000);
}
