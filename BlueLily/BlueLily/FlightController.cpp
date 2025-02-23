#include "FlightController.h"
#include "Sensors.h"
#include "Communication.h"
#include "Logger.h"
#include "Actuation.h"
#include "HID.h"

#if ENABLE_FLIGHTCONTROLLER

// Flight states
enum FlightState {
  IDLE,
  ARMED,
  ASCENT,
  APOGEE,
  DESCENT,
  LANDED
};

// State variables
static FlightState currentState = IDLE;
static unsigned long startTime = 0;
static float maxAltitude = 0.0;
static float lastAltitude = 0.0;
static uint16_t telemetrySeqNum = 0;

// Configuration thresholds
static const float LIFTOFF_ACCEL_THRESHOLD = 20.0; // m/s^2 (2g)
static const float APOGEE_VELOCITY_THRESHOLD = 0.0; // m/s (near zero at peak)
static const float LANDING_ALTITUDE_THRESHOLD = 10.0; // meters above start
static const unsigned long ARM_DELAY = 5000; // ms to arm

void initFlightController() {
  // Initialize all dependent modules (assumed called in main.ino)
  currentState = IDLE;
  startTime = 0;
  maxAltitude = 0.0;
  lastAltitude = 0.0;
  telemetrySeqNum = 0;
  Serial.println("Flight Controller Initialized");
}

void runFlightController() {
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate < LOOP_INTERVAL_MS) return;
  lastUpdate = millis();

  // Read sensor data
  float temp = readTemperature();
  float ax, ay, az, gx, gy, gz;
  readIMU(ax, ay, az, gx, gy, gz);
  int16_t adc0 = readADC(0);

  // Estimate altitude and velocity (simplified, using accel Z integration)
  static float velocity = 0.0;
  static float altitude = 0.0;
  float dt = LOOP_INTERVAL_MS / 1000.0; // Convert to seconds
  velocity += az * dt; // Integrate accel to get velocity
  altitude += velocity * dt; // Integrate velocity to get altitude

  // Update max altitude
  if (altitude > maxAltitude) maxAltitude = altitude;

  // State machine
  switch (currentState) {
    case IDLE:
      if (millis() > ARM_DELAY) { // Wait before arming
        currentState = ARMED;
        Serial.println("State: ARMED");
      }
      break;

    case ARMED:
      if (az > LIFTOFF_ACCEL_THRESHOLD) { // Detect liftoff
        currentState = ASCENT;
        startTime = micros();
        Serial.println("State: ASCENT");
      }
      break;

    case ASCENT:
      if (velocity <= APOGEE_VELOCITY_THRESHOLD && altitude > 50.0) { // Detect apogee
        currentState = APOGEE;
        setActuator(0, true); // Trigger relay (e.g., parachute)
        Serial.println("State: APOGEE");
      }
      break;

    case APOGEE:
      if (velocity < 0) { // Start descending
        currentState = DESCENT;
        Serial.println("State: DESCENT");
      }
      break;

    case DESCENT:
      if (altitude < LANDING_ALTITUDE_THRESHOLD && velocity < 5.0) { // Detect landing
        currentState = LANDED;
        Serial.println("State: LANDED");
      }
      break;

    case LANDED:
      setActuator(0, false); // Turn off relay
      flushLogger();
      syncFlashToSD();
      closeLogger();
      previewLoggedData();
      break;
  }

  // Log data
  char logBuffer[128];
  snprintf(logBuffer, sizeof(logBuffer), "%lu,%.2f,%.2f,%.2f,%.2f,%d",
           micros() - startTime, temp, az, velocity, altitude, currentState);
  logData(logBuffer);

  // Send telemetry (e.g., via LoRa)
  char telemetryBuffer[64];
  snprintf(telemetryBuffer, sizeof(telemetryBuffer), "%.2f,%.2f,%.2f,%d",
           az, velocity, altitude, currentState);
  sendLoRa('F', 1, telemetrySeqNum++, telemetryBuffer);

  // Update actuators
  runScheduler(micros() - startTime, az, temp);

  // Update HID
  updateHID();
}

#endif