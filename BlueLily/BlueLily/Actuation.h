#ifndef ACTUATION_H
#define ACTUATION_H

#include <Arduino.h>
#include "Config.h"

#if ENABLE_ACTUATION
// External declarations from Config.h
extern Actuator actuators[];
extern const uint8_t ACTUATOR_COUNT;

void initActuation();
void toggleActuator(uint8_t actuatorId);
void setActuator(uint8_t actuatorId, bool state, uint8_t pwmValue = 0);
void runScheduler(uint32_t currentTimeMicros, float accelZ, float temp);
void loadScheduleFromFlightController(uint32_t* times, uint8_t* actuatorIds, bool* states, uint8_t count);
void loadScheduleFromSD();
#else
inline void initActuation() {}
inline void toggleActuator(uint8_t) {}
inline void setActuator(uint8_t, bool, uint8_t = 0) {}
inline void runScheduler(uint32_t, float, float) {}
inline void loadScheduleFromFlightController(uint32_t*, uint8_t*, bool*, uint8_t) {}
inline void loadScheduleFromSD() {}
#endif

#endif