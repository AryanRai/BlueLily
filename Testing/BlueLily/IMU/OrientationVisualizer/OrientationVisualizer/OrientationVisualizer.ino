#include "FastIMU.h"
#include <Wire.h>

#define IMU_ADDRESS 0x68
MPU6500 IMU;
AccelData accelData;
GyroData gyroData;
MagData magData;

void setup() {
  Wire.begin();
  Wire.setClock(400000);
  Serial.begin(115200);

  int err = IMU.init({}, IMU_ADDRESS);
  if (err != 0) {
    Serial.print("Error initializing IMU: ");
    Serial.println(err);
    while (true) {}
  }
}

void loop() {
  IMU.update();
  IMU.getAccel(&accelData);
  IMU.getGyro(&gyroData);
  
  Serial.print(accelData.accelX); Serial.print(",");
  Serial.print(accelData.accelY); Serial.print(",");
  Serial.print(accelData.accelZ); Serial.print(",");
  Serial.print(gyroData.gyroX); Serial.print(",");
  Serial.print(gyroData.gyroY); Serial.print(",");
  Serial.print(gyroData.gyroZ);

  if (IMU.hasMagnetometer()) {
    IMU.getMag(&magData);
    Serial.print(",");
    Serial.print(magData.magX); Serial.print(",");
    Serial.print(magData.magY); Serial.print(",");
    Serial.print(magData.magZ);
  } else {
    Serial.print(",,,");
  }

  if (IMU.hasTemperature()) {
    Serial.print(",");
    Serial.print(IMU.getTemp());
  }

  Serial.println();
  delay(50);
}
