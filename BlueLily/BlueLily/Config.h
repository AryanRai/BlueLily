#ifndef CONFIG_H
#define CONFIG_H

// Sensor Enable/Disable Flags
#define ENABLE_MAX31855 1
#define ENABLE_MPU6500  1
#define ENABLE_ADS1115  1

// Sensor Pins and Addresses
#if ENABLE_ADS1115
#define ADS1115_I2C_ADDR 0x48
#endif
#if ENABLE_MPU6500
#define MPU6500_I2C_ADDR 0x68
#endif
#if ENABLE_MAX31855
#define MAX31855_CS_PIN 10
#endif

// Communication Enable/Disable Flags
#define ENABLE_RS485    1
#define ENABLE_CANBUS   1
#define ENABLE_BLUETOOTH 1
#define ENABLE_LORA     0

// Communication Pins
#if ENABLE_RS485
#define RS485_TX_EN_PIN 6
#define RS485_RX_PIN 7
#define RS485_TX_PIN 8
#endif
#if ENABLE_CANBUS
#define CAN_CS_PIN 9
#endif
#if ENABLE_BLUETOOTH
#define BLUETOOTH_RX_PIN 0
#define BLUETOOTH_TX_PIN 1
#endif
#if ENABLE_LORA
#define LORA_SS_PIN 15
#define LORA_RST_PIN 16
#define LORA_DIO0_PIN 17
#endif

// Communication Settings
#if ENABLE_RS485
#define RS485_BAUD 115200
#endif
#if ENABLE_CANBUS
#define CANBUS_BAUD CAN_500KBPS
#endif
#if ENABLE_BLUETOOTH
#define BLUETOOTH_BAUD 9600
#endif
#if ENABLE_LORA
#define LORA_FREQ 433E6
#endif

// chYAPpy v1.2 Constants
#define CHYAPPY_V1_2_START 0x7D
#define PAYLOAD_TYPE_STRING 0x01
#define PAYLOAD_TYPE_FLOAT 0x02
#define PAYLOAD_TYPE_INT16 0x03
#define PAYLOAD_TYPE_INT32 0x04
#define SENSOR_TYPE_CONFIG 'C'
#define SENSOR_TYPE_ACK 'A'

// Configurator Constants
#define CONFIG_BUFFER_SIZE 64
#define MAX_SETTINGS 10
#define KEY_MAX_LEN 16
#define VALUE_MAX_LEN 32

// Logger Enable/Disable Flags
#define ENABLE_SD       1
#define ENABLE_W25Q128  1

// Logger Settings
#if ENABLE_SD
#define SD_CONFIG SdioConfig(FIFO_SDIO)
#define SD_LOG_FILE_SIZE 150000000
#define RING_BUF_CAPACITY 400*512
#endif
#if ENABLE_W25Q128
#define W25Q128_CS_PIN 25
#define W25Q128_CAPACITY 16777216
#endif

// Actuation Enable/Disable Flag
#define ENABLE_ACTUATION 1

#if ENABLE_ACTUATION
#define MAX_SCHEDULE_EVENTS 10

enum ActuatorType {
  RELAY = 0,
  PWM
};

struct Actuator {
  uint8_t id;
  uint8_t pin;
  ActuatorType type;
  bool state;
  uint8_t pwmValue;
};

static Actuator actuators[] = {
  {0, 21, RELAY, false, 0}, // Relay on pin 21
  {1, 29, PWM,   false, 0}  // PWM actuator on pin 29
};
#endif

const uint8_t ACTUATOR_COUNT =
#if ENABLE_ACTUATION
  sizeof(actuators) / sizeof(actuators[0]);
#else
  0;
#endif

// HID Enable/Disable Flag
#define ENABLE_HID 1

// HID Pins
#if ENABLE_HID
#define POT_PIN 17
#define BTN_SELECT 26
#define BTN_BACK 27
#define LED_1_PIN 23  // Red
#define LED_2_PIN 22 // Yellow 
#define LED_3_PIN 37  // Green
#define LED_4_PIN 36  // Blue
#endif

// Timing
#define LOOP_INTERVAL_MS 50

#endif

// FlightController Enable/Disable Flag
#define ENABLE_FLIGHTCONTROLLER 1