# BlueLily
A High Performance Customizable Flight Computer for High Performance Rocketry, Payload, CubeSats.

### **Project Goal**  
The goal of this project is to develop a **flight computer** for **high-powered and high-performance rocketry**. The system will:  
- **Collect real-time sensor data** (temperature, IMU, ADC readings).  
- **Enable robust communication** via **RS485, CANBUS, Bluetooth, and LoRa**.  
- **Log critical flight data** to an **SD card** for post-flight analysis.  
- **Control actuators** (solid-state relay) for mission-critical operations.  
- **Process user inputs** (buttons, potentiometer) and provide visual feedback (LEDs).  
- **Ensure reliability** with efficient data handling and modular design.  

---

The project involves a Teensy 4.1 as the main controller, integrating various sensor modules, communication protocols, storage, actuation, and user input/output components:

### **Sensors:**
1. **MAX31855** – A thermocouple module from Adafruit for temperature sensing.
2. **MPU9250/MPU6500** – Inertial Measurement Units (IMUs) for motion and orientation tracking.
3. **ADS1115** – A 16-bit ADC for precise analog signal measurements.

### **Communication:**
1. **RS485 (MAX485 breakout board)** – For robust, long-distance serial communication.
2. **CANBUS (MCP2515)** – For vehicle/industrial-grade networking between multiple microcontrollers.
3. **Bluetooth (HC-05)** – For wireless communication with other devices.
4. **LoRa (SX1278)** – For long-range, low-power wireless data transmission.

### **Storage:**
- Utilizing **Teensy's built-in SD card** for logging data.

### **Actuation:**
- **10A solid-state relay** to control high-power loads safely.

### **Input/Output:**
1. **4 buttons** – Likely used for user interaction or control.
2. **4 LEDs** – Providing visual feedback or status indication.
3. **1 potentiometer** – Analog control for adjusting parameters.

---