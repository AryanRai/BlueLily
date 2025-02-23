#include "Actuation.h"

#if ENABLE_ACTUATION

// Condition types
enum ConditionType {
  NONE = 0,
  ACCEL_Z,
  TEMP
};

// Schedule event
struct ScheduleEvent {
  uint32_t timeMicros;
  uint8_t actuatorId;
  bool state;
  uint8_t pwmValue;
  ConditionType conditionType;
  float conditionValue;
  bool conditionGreater;
  bool triggered;
};

// Default hardcoded schedule
static ScheduleEvent schedule[] = {
  {5000000,  0, true,  0,   NONE,    0.0,  false, false}, // 5s, relay 0 on
  {10000000, 0, false, 0,   NONE,    0.0,  false, false}, // 10s, relay 0 off
  {15000000, 1, true,  128, ACCEL_Z, -10.0, false, false}, // 15s or accelZ < -10, PWM 1 to 128
  {20000000, 1, true,  255, TEMP,    50.0, true,  false}  // 20s or temp > 50°C, PWM 1 to 255
};
static uint8_t scheduleCount = sizeof(schedule) / sizeof(schedule[0]);

void initActuation() {
  for (uint8_t i = 0; i < ACTUATOR_COUNT; i++) {
    pinMode(actuators[i].pin, OUTPUT);
    if (actuators[i].type == RELAY) {
      digitalWrite(actuators[i].pin, LOW);
      actuators[i].state = false;
    } else if (actuators[i].type == PWM) {
      analogWrite(actuators[i].pin, 0);
      actuators[i].pwmValue = 0;
    }
  }
  Serial.println("Actuation Initialized");
}

void toggleActuator(uint8_t actuatorId) {
  for (uint8_t i = 0; i < ACTUATOR_COUNT; i++) {
    if (actuators[i].id == actuatorId) {
      if (actuators[i].type == RELAY) {
        actuators[i].state = !actuators[i].state;
        digitalWrite(actuators[i].pin, actuators[i].state);
        Serial.print("Relay ");
        Serial.print(actuatorId);
        Serial.print(" toggled to: ");
        Serial.println(actuators[i].state ? "HIGH" : "LOW");
      } else if (actuators[i].type == PWM) {
        actuators[i].pwmValue = (actuators[i].pwmValue == 0) ? 128 : 0;
        analogWrite(actuators[i].pin, actuators[i].pwmValue);
        Serial.print("PWM ");
        Serial.print(actuatorId);
        Serial.print(" toggled to: ");
        Serial.println(actuators[i].pwmValue);
      }
      return;
    }
  }
  Serial.println("Invalid actuator ID");
}

void setActuator(uint8_t actuatorId, bool state, uint8_t pwmValue) {
  for (uint8_t i = 0; i < ACTUATOR_COUNT; i++) {
    if (actuators[i].id == actuatorId) {
      if (actuators[i].type == RELAY) {
        actuators[i].state = state;
        digitalWrite(actuators[i].pin, state);
      } else if (actuators[i].type == PWM) {
        actuators[i].state = (pwmValue > 0);
        actuators[i].pwmValue = pwmValue;
        analogWrite(actuators[i].pin, pwmValue);
      }
      return;
    }
  }
}

void runScheduler(uint32_t currentTimeMicros, float accelZ, float temp) {
  for (uint8_t i = 0; i < scheduleCount; i++) {
    if (!schedule[i].triggered) {
      bool trigger = false;
      switch (schedule[i].conditionType) {
        case NONE:
          trigger = (currentTimeMicros >= schedule[i].timeMicros);
          break;
        case ACCEL_Z:
          trigger = schedule[i].conditionGreater ? (accelZ > schedule[i].conditionValue) : (accelZ < schedule[i].conditionValue);
          break;
        case TEMP:
          trigger = schedule[i].conditionGreater ? (temp > schedule[i].conditionValue) : (temp < schedule[i].conditionValue);
          break;
      }
      if (trigger) {
        setActuator(schedule[i].actuatorId, schedule[i].state, schedule[i].pwmValue);
        schedule[i].triggered = true;
        Serial.print("Scheduled event triggered: Actuator ");
        Serial.print(schedule[i].actuatorId);
        if (actuators[schedule[i].actuatorId].type == RELAY) {
          Serial.print(" set to ");
          Serial.println(schedule[i].state ? "ON" : "OFF");
        } else {
          Serial.print(" PWM set to ");
          Serial.println(schedule[i].pwmValue);
        }
      }
    }
  }
}

void loadScheduleFromFlightController(uint32_t* times, uint8_t* actuatorIds, bool* states, uint8_t count) {
  if (count > MAX_SCHEDULE_EVENTS) {
    Serial.println("Schedule exceeds max events");
    count = MAX_SCHEDULE_EVENTS;
  }
  scheduleCount = count;
  for (uint8_t i = 0; i < count; i++) {
    schedule[i].timeMicros = times[i];
    schedule[i].actuatorId = actuatorIds[i];
    schedule[i].state = states[i];
    schedule[i].pwmValue = 0;
    schedule[i].conditionType = NONE;
    schedule[i].triggered = false;
  }
  Serial.println("Schedule loaded from FlightController");
}

void loadScheduleFromSD() {
  Serial.println("SD schedule loading not implemented yet");
}
#endif