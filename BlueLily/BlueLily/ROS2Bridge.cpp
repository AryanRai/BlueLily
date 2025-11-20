#include "ROS2Bridge.h"

#if ENABLE_ROS2_BRIDGE

// Timing variables
static unsigned long lastIMUPublish = 0;
static unsigned long lastHeartbeat = 0;

// Sequence counter for message tracking
static uint32_t messageSequence = 0;

void initROS2Bridge() {
  ROS2_SERIAL.begin(ROS2_BAUD);
  
  // Wait for serial to be ready
  delay(100);
  
  // Send initialization message
  ROS2_SERIAL.println("# BlueLily ROS2 Bridge Initialized");
  ROS2_SERIAL.print("# Firmware Version: ");
  ROS2_SERIAL.println("1.0.0");
  ROS2_SERIAL.print("# IMU Rate: ");
  ROS2_SERIAL.print(1000 / ROS2_PUBLISH_RATE_MS);
  ROS2_SERIAL.println(" Hz");
  ROS2_SERIAL.println("# Message Format: TYPE,timestamp,seq,data...");
  ROS2_SERIAL.println("# Ready");
}

void publishIMU(float accelX, float accelY, float accelZ, 
                float gyroX, float gyroY, float gyroZ) {
  // Format: IMU,timestamp,seq,ax,ay,az,gx,gy,gz
  ROS2_SERIAL.print("IMU,");
  ROS2_SERIAL.print(millis());
  ROS2_SERIAL.print(",");
  ROS2_SERIAL.print(messageSequence++);
  ROS2_SERIAL.print(",");
  ROS2_SERIAL.print(accelX, 6);
  ROS2_SERIAL.print(",");
  ROS2_SERIAL.print(accelY, 6);
  ROS2_SERIAL.print(",");
  ROS2_SERIAL.print(accelZ, 6);
  ROS2_SERIAL.print(",");
  ROS2_SERIAL.print(gyroX, 6);
  ROS2_SERIAL.print(",");
  ROS2_SERIAL.print(gyroY, 6);
  ROS2_SERIAL.print(",");
  ROS2_SERIAL.println(gyroZ, 6);
}

void publishTemperature(float temperature) {
  // Format: TEMP,timestamp,seq,temperature
  ROS2_SERIAL.print("TEMP,");
  ROS2_SERIAL.print(millis());
  ROS2_SERIAL.print(",");
  ROS2_SERIAL.print(messageSequence++);
  ROS2_SERIAL.print(",");
  ROS2_SERIAL.println(temperature, 2);
}

void publishADC(float voltages[4]) {
  // Format: ADC,timestamp,seq,ch0,ch1,ch2,ch3
  ROS2_SERIAL.print("ADC,");
  ROS2_SERIAL.print(millis());
  ROS2_SERIAL.print(",");
  ROS2_SERIAL.print(messageSequence++);
  for (int i = 0; i < 4; i++) {
    ROS2_SERIAL.print(",");
    ROS2_SERIAL.print(voltages[i], 4);
  }
  ROS2_SERIAL.println();
}

void publishState(const char* stateName) {
  // Format: STATE,timestamp,seq,state_name
  ROS2_SERIAL.print("STATE,");
  ROS2_SERIAL.print(millis());
  ROS2_SERIAL.print(",");
  ROS2_SERIAL.print(messageSequence++);
  ROS2_SERIAL.print(",");
  ROS2_SERIAL.println(stateName);
}

void publishHeartbeat() {
  // Format: HEARTBEAT,timestamp,seq
  ROS2_SERIAL.print("HEARTBEAT,");
  ROS2_SERIAL.print(millis());
  ROS2_SERIAL.print(",");
  ROS2_SERIAL.println(messageSequence++);
}

void receiveROS2Commands() {
  // Check for incoming commands
  if (ROS2_SERIAL.available()) {
    String command = ROS2_SERIAL.readStringUntil('\n');
    command.trim();
    
    // Parse command
    // Format: CMD,command_type,parameters
    if (command.startsWith("CMD,")) {
      // Remove "CMD," prefix
      command = command.substring(4);
      
      // Send acknowledgment
      ROS2_SERIAL.print("ACK,");
      ROS2_SERIAL.print(millis());
      ROS2_SERIAL.print(",");
      ROS2_SERIAL.println(command);
      
      // TODO: Process commands (actuator control, config changes, etc.)
      // This can be integrated with existing Configurator module
    }
  }
}

void updateROS2Bridge() {
  unsigned long currentTime = millis();
  
  // Publish IMU at configured rate (default 100Hz)
  if (currentTime - lastIMUPublish >= ROS2_PUBLISH_RATE_MS) {
    #if ENABLE_MPU6500
    float ax, ay, az, gx, gy, gz;
    readIMU(ax, ay, az, gx, gy, gz);
    publishIMU(ax, ay, az, gx, gy, gz);
    #endif
    lastIMUPublish = currentTime;
  }
  
  // Publish heartbeat every second
  if (currentTime - lastHeartbeat >= 1000) {
    publishHeartbeat();
    lastHeartbeat = currentTime;
  }
  
  // Check for incoming commands
  receiveROS2Commands();
}

#endif // ENABLE_ROS2_BRIDGE
