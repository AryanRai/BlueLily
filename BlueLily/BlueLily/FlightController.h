#ifndef FLIGHTCONTROLLER_H
#define FLIGHTCONTROLLER_H

#include <Arduino.h>
#include "Config.h"

#if ENABLE_FLIGHTCONTROLLER
void initFlightController();
void runFlightController();
#else
inline void initFlightController() {}
inline void runFlightController() {}
#endif

#endif