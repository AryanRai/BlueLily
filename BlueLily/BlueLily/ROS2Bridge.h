#ifndef ROS2BRIDGE_H
#define ROS2BRIDGE_H

#include <Arduino.h>
#include "Config.h"

// ROS2 Bridge Enable/Disable (add this to Config.h later)
#ifndef ENABLE_ROS2_BRIDGE
#define ENABLE_ROS2_BRIDGE 1
#endif

#if ENABLE_ROS2_BRIDGE

// ROS2 Bridge Settings
#define ROS2_SERIAL Serial  // Use main USB serial
#define ROS2_BAUD 115200
#define ROS2_PUBLISH_RATE_MS 10  // 100Hz IMU publishing

// Message types
enum ROS2MessageType {
  ROS2_IMU = 0,
  ROS2_TEMP = 1,
  ROS2_ADC = 2,
  ROS2_STATE = 3,
  ROS2_HEARTBEAT = 4
};

/**
 * Initialize ROS2 bridge
 * Sets up serial communication for ROS2 integration
 */
void initROS2Bridge();

/**
 * Publish IMU data to ROS2
 * Format: IMU,timestamp,ax,ay,az,gx,gy,gz
 * 
 * @param accelX Acceleration X (m/s²)
 * @param accelY Acceleration Y (m/s²)
 * @param accelZ Acceleration Z (m/s²)
 * @param gyroX Gyroscope X (rad/s)
 * @param gyroY Gyroscope Y (rad/s)
 * @param gyroZ Gyroscope Z (rad/s)
 */
void publishIMU(float accelX, float accelY, float accelZ, 
                float gyroX, float gyroY, float gyroZ);

/**
 * Publish temperature data to ROS2
 * Format: TEMP,timestamp,temperature
 * 
 * @param temperature Temperature in Celsius
 */
void publishTemperature(float temperature);

/**
 * Publish ADC voltage data to ROS2
 * Format: ADC,timestamp,ch0,ch1,ch2,ch3
 * 
 * @param voltages Array of 4 voltage readings
 */
void publishADC(float voltages[4]);

/**
 * Publish flight state to ROS2
 * Format: STATE,timestamp,state_name
 * 
 * @param stateName Current flight state (IDLE, ARMED, ASCENT, etc.)
 */
void publishState(const char* stateName);

/**
 * Publish heartbeat to ROS2
 * Format: HEARTBEAT,timestamp
 */
void publishHeartbeat();

/**
 * Check for incoming ROS2 commands
 * Processes commands like actuator control, configuration changes
 */
void receiveROS2Commands();

/**
 * Update ROS2 bridge (call in main loop)
 * Handles periodic publishing and command reception
 */
void updateROS2Bridge();

#else
// Stub functions when ROS2 bridge is disabled
inline void initROS2Bridge() {}
inline void publishIMU(float, float, float, float, float, float) {}
inline void publishTemperature(float) {}
inline void publishADC(float[4]) {}
inline void publishState(const char*) {}
inline void publishHeartbeat() {}
inline void receiveROS2Commands() {}
inline void updateROS2Bridge() {}
#endif

#endif // ROS2BRIDGE_H
