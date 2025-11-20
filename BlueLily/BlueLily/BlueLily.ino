#include "Config.h"
#include "Sensors.h"
#include "Communication.h"
#include "Logger.h"
#include "Actuation.h"
#include "HID.h"
#include "FlightController.h"
#include "ROS2Bridge.h"

void setup() {
  Serial.begin(115200);
  delay(1000);

  initSensors();
  initCommunication();
  initLogger();
  initActuation();
  initHID();
  initFlightController();
  
  #if ENABLE_ROS2_BRIDGE
  initROS2Bridge();
  #endif

  Serial.println("BlueLily Flight Computer Initialized");
}

void loop() {
  runFlightController();
  
  #if ENABLE_ROS2_BRIDGE
  updateROS2Bridge();
  #endif
}