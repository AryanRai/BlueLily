#include "Config.h"
#include "Sensors.h"
#include "Communication.h"
#include "Logger.h"
#include "Actuation.h"
#include "HID.h"
#include "FlightController.h"

void setup() {
  Serial.begin(115200);
  delay(1000);

  initSensors();
  initCommunication();
  initLogger();
  initActuation();
  initHID();
  initFlightController();

  Serial.println("BlueLily Flight Computer Initialized");
}

void loop() {
  runFlightController();
}