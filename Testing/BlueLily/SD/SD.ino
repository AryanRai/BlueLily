#include "SdFat.h"

SdExFat sd;

void setup()
{
  if (!sd.begin(SdioConfig(FIFO_SDIO))) { // SdFat.h Ver 2.0.0 Beta
    Serial.println("m.SD initialization failed!");
  } else {
    Serial.println("m.SD initialization OK");
  }

  auto file = sd.open("hi.txt", FILE_WRITE);
  file.write("hi");
  file.close();
}

void loop()
{
  
}