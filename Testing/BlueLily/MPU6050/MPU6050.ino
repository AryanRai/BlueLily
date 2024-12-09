#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

// Create an MPU6050 object
Adafruit_MPU6050 mpu;

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);  // Wait for Serial to initialize
  Serial.println("Initializing MPU...");

  // Initialize I2C and MPU6050
  if (!mpu.begin(0x68)) {  // Use 0x68 if AD0 is GND, 0x69 if AD0 is 3.3V
    Serial.println("Failed to find MPU6050 chip. Check wiring!");
    while (1) delay(10);
  }

  Serial.println("MPU6050 Found!");
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);  // Set accelerometer range
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);      // Set gyroscope range
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);   // Set filter bandwidth

  delay(100);
}

void loop() {
  /* Get new sensor events with the Adafruit Unified Sensor API */
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  // Print Accelerometer Data
  Serial.print("Accel X: "); Serial.print(a.acceleration.x); Serial.print(" m/s^2, ");
  Serial.print("Y: "); Serial.print(a.acceleration.y); Serial.print(" m/s^2, ");
  Serial.print("Z: "); Serial.print(a.acceleration.z); Serial.println(" m/s^2");

  // Print Gyroscope Data
  Serial.print("Gyro X: "); Serial.print(g.gyro.x); Serial.print(" rad/s, ");
  Serial.print("Y: "); Serial.print(g.gyro.y); Serial.print(" rad/s, ");
  Serial.print("Z: "); Serial.print(g.gyro.z); Serial.println(" rad/s");

  // Print Temperature
  Serial.print("Temperature: "); Serial.print(temp.temperature); Serial.println(" deg C");

  Serial.println("-----------------------------------");
  delay(500);  // Delay for readability
}
