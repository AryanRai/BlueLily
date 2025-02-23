#ifndef HID_H
#define HID_H

#include <Arduino.h>
#include "Config.h"

#if ENABLE_HID
void initHID();
void updateHID();
void drawStatusIcons(bool bluetooth, bool rs485, bool canbus, bool lora);
#else
inline void initHID() {}
inline void updateHID() {}
inline void drawStatusIcons(bool, bool, bool, bool) {}
#endif

#endif