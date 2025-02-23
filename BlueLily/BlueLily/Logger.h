#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>
#include "Config.h"

void initLogger();
void logData(const char* data);
void flushLogger();
void closeLogger();
void previewLoggedData();
void syncFlashToSD(); // Declaration added

#endif