#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SdFat.h>
#include "HID.h"
#include "Sensors.h"
#include "Communication.h"
#include "Logger.h"
#include "Actuation.h"

#if ENABLE_HID

// Icons
const uint8_t bluetoothIcon[] PROGMEM = {0b00011000, 0b00100100, 0b01000100, 0b10001000, 0b01000100, 0b00100100, 0b00011000, 0b00000000};
const uint8_t rs485Icon[] PROGMEM = {0b00011000, 0b00111100, 0b01111110, 0b01100110, 0b01100110, 0b01111110, 0b00111100, 0b00011000};
const uint8_t canbusIcon[] PROGMEM = {0b00111100, 0b01000010, 0b10011001, 0b10111101, 0b10111101, 0b10011001, 0b01000010, 0b00111100};
const uint8_t loraIcon[] PROGMEM = {0b00001000, 0b00010100, 0b00100010, 0b01000001, 0b01000001, 0b00100010, 0b00010100, 0b00001000};
const uint8_t rocketBitmap[] PROGMEM = {0b00011000, 0b00111100, 0b01111110, 0b11111111, 0b01111110, 0b00111100, 0b00011000, 0b00011000};

// Display setup
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Menu Definitions
const char* mainMenuItems[] = {"Sensors", "Communication", "Logger", "Actuation", "HID", "FlightController", "Config"};
const char* sensorsMenu[] = {"MAX31855", "MPU6500", "ADS1115", "Back"};
const char* commMenu[] = {"RS485", "CANBUS", "Bluetooth", "LoRa", "Back"};
const char* loggerMenu[] = {"SD", "W25Q128", "Back"};
const char* actuationMenu[] = {"Relay 0", "PWM 1", "Back"};
const char* hidMenu[] = {"Display", "Potentiometer", "Buttons", "Back"};
const char* flightMenu[] = {"State", "Back"};
const char* configMenu[] = {"Timeouts", "Back"};
const char* hardwareSettings[] = {"Enable/Disable", "Preview", "Back"};

const int mainMenuSize = sizeof(mainMenuItems) / sizeof(mainMenuItems[0]);
const int sensorsMenuSize = sizeof(sensorsMenu) / sizeof(sensorsMenu[0]);
const int commMenuSize = sizeof(commMenu) / sizeof(commMenu[0]);
const int loggerMenuSize = sizeof(loggerMenu) / sizeof(loggerMenu[0]);
const int actuationMenuSize = sizeof(actuationMenu) / sizeof(actuationMenu[0]);
const int hidMenuSize = sizeof(hidMenu) / sizeof(hidMenu[0]);
const int flightMenuSize = sizeof(flightMenu) / sizeof(flightMenu[0]);
const int configMenuSize = sizeof(configMenu) / sizeof(configMenu[0]);
const int settingsMenuSize = sizeof(hardwareSettings) / sizeof(hardwareSettings[0]);

// State variables
static int menuIndex = 0;
static int subMenuLevel = 0;
static const char** currentMenu = mainMenuItems;
static int currentMenuSize = mainMenuSize;
static int lastPotValue = 0;
static unsigned long lastInteractionTime = 0;
static const unsigned long screensaverTimeout = 30000;
static const int deadbandThreshold = 50;
static bool inScreensaver = false;
static bool inPreview = false;
static int selectedModule = -1;
static int selectedHardware = -1;

// Scrolling variables
static int scrollOffset = 0;
static int menuScrollOffset = 0;
static unsigned long lastScrollTime = 0;
static const unsigned long scrollDelay = 300;
static int scrollbarHeight = 20;
static int scrollbarX = SCREEN_WIDTH - 5;
static const int itemsPerPage = 4;

// Boot animation variables
static const int ANIMATION_DELAY = 200;

// Screensaver Animation
static int rocketY = SCREEN_HEIGHT - 10;
static const int rocketX = SCREEN_WIDTH / 2 - 4;

// Enable/Disable states
static bool max31855Enabled = true;
static bool mpu6500Enabled = true;
static bool ads1115Enabled = true;
static bool rs485Enabled = true;
static bool canbusEnabled = false;
static bool bluetoothEnabled = true;
static bool loraEnabled = true;
static bool sdEnabled = true;
static bool w25q128Enabled = true;
static bool relay0Enabled = true;
static bool pwm1Enabled = true;

// Communication stats (placeholder)
static uint32_t rs485Packets = 0;
static uint32_t canbusPackets = 0;
static uint32_t bluetoothPackets = 0;
static uint32_t loraPackets = 0;

// Forward declarations
static void handleInput();
static void checkForInteraction();
static void drawMenu();
static void drawScreensaver();
static void drawPreview();
static void toggleEnableDisable();
static void saveSettingsToSD();
static void loadSettingsFromSD();
static void bootAnimation();

void initHID() {
  pinMode(BTN_SELECT, INPUT_PULLUP);
  pinMode(BTN_BACK, INPUT_PULLUP);
  pinMode(LED_1_PIN, OUTPUT);
  pinMode(LED_2_PIN, OUTPUT);
  pinMode(LED_3_PIN, OUTPUT);
  pinMode(LED_4_PIN, OUTPUT);

  bootAnimation(); // Run boot animation before display init

  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    while (1);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.display();
  lastInteractionTime = millis();
  loadSettingsFromSD();
  Serial.println("HID Initialized");
}

void updateHID() {
  if (inScreensaver) {
    drawScreensaver();
    checkForInteraction();
  } else if (inPreview) {
    drawPreview();
    checkForInteraction();
  } else {
    handleInput();
    drawMenu();
  }
}

static void handleInput() {
  int potValue = analogRead(POT_PIN);
  int mappedIndex = map(potValue, 0, 1023, 0, currentMenuSize - 1);

  if (mappedIndex != menuIndex) {
    menuIndex = mappedIndex;
    lastInteractionTime = millis();
    scrollOffset = 0;
    if (menuIndex < menuScrollOffset) {
      menuScrollOffset = menuIndex;
    } else if (menuIndex >= menuScrollOffset + itemsPerPage) {
      menuScrollOffset = menuIndex - itemsPerPage + 1;
    }
  }

  lastPotValue = potValue;

  if (digitalRead(BTN_SELECT) == LOW) {
    lastInteractionTime = millis();
    if (subMenuLevel == 0) {
      selectedModule = menuIndex;
      switch (menuIndex) {
        case 0: currentMenu = sensorsMenu; currentMenuSize = sensorsMenuSize; break;
        case 1: currentMenu = commMenu; currentMenuSize = commMenuSize; break;
        case 2: currentMenu = loggerMenu; currentMenuSize = loggerMenuSize; break;
        case 3: currentMenu = actuationMenu; currentMenuSize = actuationMenuSize; break;
        case 4: currentMenu = hidMenu; currentMenuSize = hidMenuSize; break;
        case 5: currentMenu = flightMenu; currentMenuSize = flightMenuSize; break;
        case 6: currentMenu = configMenu; currentMenuSize = configMenuSize; break;
      }
      subMenuLevel = 1;
      menuIndex = 0;
      menuScrollOffset = 0;
    } else if (subMenuLevel == 1) {
      if (menuIndex == currentMenuSize - 1) {
        currentMenu = mainMenuItems;
        currentMenuSize = mainMenuSize;
        subMenuLevel = 0;
        menuIndex = selectedModule;
        menuScrollOffset = 0;
      } else {
        selectedHardware = menuIndex;
        currentMenu = hardwareSettings;
        currentMenuSize = settingsMenuSize;
        subMenuLevel = 2;
        menuIndex = 0;
        menuScrollOffset = 0;
      }
    } else if (subMenuLevel == 2) {
      if (menuIndex == 2) {
        subMenuLevel = 1;
        menuIndex = selectedHardware;
        switch (selectedModule) {
          case 0: currentMenu = sensorsMenu; currentMenuSize = sensorsMenuSize; break;
          case 1: currentMenu = commMenu; currentMenuSize = commMenuSize; break;
          case 2: currentMenu = loggerMenu; currentMenuSize = loggerMenuSize; break;
          case 3: currentMenu = actuationMenu; currentMenuSize = actuationMenuSize; break;
          case 4: currentMenu = hidMenu; currentMenuSize = hidMenuSize; break;
          case 5: currentMenu = flightMenu; currentMenuSize = flightMenuSize; break;
          case 6: currentMenu = configMenu; currentMenuSize = configMenuSize; break;
        }
      } else if (menuIndex == 0) {
        toggleEnableDisable();
        saveSettingsToSD();
      } else if (menuIndex == 1) {
        inPreview = true;
        scrollOffset = 0;
      }
    }
    delay(300);
  }

  if (digitalRead(BTN_BACK) == LOW && subMenuLevel > 0) {
    lastInteractionTime = millis();
    if (subMenuLevel == 2) {
      subMenuLevel = 1;
      menuIndex = selectedHardware;
      switch (selectedModule) {
        case 0: currentMenu = sensorsMenu; currentMenuSize = sensorsMenuSize; break;
        case 1: currentMenu = commMenu; currentMenuSize = commMenuSize; break;
        case 2: currentMenu = loggerMenu; currentMenuSize = loggerMenuSize; break;
        case 3: currentMenu = actuationMenu; currentMenuSize = actuationMenuSize; break;
        case 4: currentMenu = hidMenu; currentMenuSize = hidMenuSize; break;
        case 5: currentMenu = flightMenu; currentMenuSize = flightMenuSize; break;
        case 6: currentMenu = configMenu; currentMenuSize = configMenuSize; break;
      }
    } else {
      currentMenu = mainMenuItems;
      currentMenuSize = mainMenuSize;
      subMenuLevel = 0;
      menuIndex = selectedModule;
    }
    delay(300);
  }

  if (millis() - lastInteractionTime > screensaverTimeout) {
    inScreensaver = true;
  }
}

static void toggleEnableDisable() {
  switch (selectedModule) {
    case 0: // Sensors
      if (selectedHardware == 0) max31855Enabled = !max31855Enabled;
      else if (selectedHardware == 1) mpu6500Enabled = !mpu6500Enabled;
      else if (selectedHardware == 2) ads1115Enabled = !ads1115Enabled;
      break;
    case 1: // Communication
      if (selectedHardware == 0) rs485Enabled = !rs485Enabled;
      else if (selectedHardware == 1) canbusEnabled = !canbusEnabled;
      else if (selectedHardware == 2) bluetoothEnabled = !bluetoothEnabled;
      else if (selectedHardware == 3) loraEnabled = !loraEnabled;
      break;
    case 2: // Logger
      if (selectedHardware == 0) sdEnabled = !sdEnabled;
      else if (selectedHardware == 1) w25q128Enabled = !w25q128Enabled;
      break;
    case 3: // Actuation
      if (selectedHardware == 0) relay0Enabled = !relay0Enabled;
      else if (selectedHardware == 1) pwm1Enabled = !pwm1Enabled;
      break;
  }
}

static void checkForInteraction() {
  int potValue = analogRead(POT_PIN);

  if (abs(potValue - lastPotValue) > deadbandThreshold ||
      digitalRead(BTN_SELECT) == LOW ||
      digitalRead(BTN_BACK) == LOW) {
    inScreensaver = false;
    inPreview = false;
    lastInteractionTime = millis();
    delay(300);
  }

  lastPotValue = potValue;
}

static void drawMenu() {
  display.clearDisplay();

  display.setTextSize(1);
  display.setCursor(10, 2);
  if (subMenuLevel == 0) display.print("Main Menu");
  else if (subMenuLevel == 1) display.print(mainMenuItems[selectedModule]);
  else display.print(currentMenu[selectedHardware]);

  drawStatusIcons(bluetoothEnabled, rs485Enabled, canbusEnabled, loraEnabled);

  for (int i = menuScrollOffset; i < min(menuScrollOffset + itemsPerPage, currentMenuSize); i++) {
    int yPos = 20 + ((i - menuScrollOffset) * 10);
    display.setCursor(10, yPos);
    if (i == menuIndex) {
      display.print("> ");
    } else {
      display.print("  ");
    }
    display.print(currentMenu[i]);
  }

  if (currentMenuSize > itemsPerPage) {
    int scrollBarMaxY = SCREEN_HEIGHT - scrollbarHeight - 5;
    int scrollBarPosition = map(menuIndex, 0, currentMenuSize - 1, 10, scrollBarMaxY);
    display.fillRect(scrollbarX, scrollBarPosition, 3, scrollbarHeight, SSD1306_WHITE);
  }

  display.display();
}

static void drawScreensaver() {
  display.clearDisplay();

  drawStatusIcons(bluetoothEnabled, rs485Enabled, canbusEnabled, loraEnabled);
  display.setCursor(0, 0);
  display.print("GodSpeed");
  display.setCursor(40, 30);
  display.print("Bluelily...");

  display.drawBitmap(rocketX, rocketY, rocketBitmap, 8, 8, SSD1306_WHITE);

  rocketY -= 1;
  if (rocketY < -10) {
    rocketY = SCREEN_HEIGHT - 10;
  }

  display.display();
  delay(50);
}

static void drawPreview() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Preview: ");
  display.print(currentMenu[selectedHardware]);

  char buffer[64];
  int textWidth;
  switch (selectedModule) {
    case 0: // Sensors
      if (selectedHardware == 0 && max31855Enabled) {
        snprintf(buffer, sizeof(buffer), "Temp: %.2f C", readTemperature());
      } else if (selectedHardware == 1 && mpu6500Enabled) {
        float ax, ay, az, gx, gy, gz;
        readIMU(ax, ay, az, gx, gy, gz);
        snprintf(buffer, sizeof(buffer), "AX:%.1f AY:%.1f AZ:%.1f GX:%.1f GY:%.1f GZ:%.1f", ax, ay, az, gx, gy, gz);
      } else if (selectedHardware == 2 && ads1115Enabled) {
        snprintf(buffer, sizeof(buffer), "V0: %.3f V1: %.3f V2: %.3f V3: %.3f",
                 readADCVoltage(0), readADCVoltage(1), readADCVoltage(2), readADCVoltage(3));
      } else {
        snprintf(buffer, sizeof(buffer), "Disabled");
      }
      break;
    case 1: // Communication
      if (selectedHardware == 0 && rs485Enabled) snprintf(buffer, sizeof(buffer), "Packets: %lu", rs485Packets++);
      else if (selectedHardware == 1 && canbusEnabled) snprintf(buffer, sizeof(buffer), "Packets: %lu", canbusPackets++);
      else if (selectedHardware == 2 && bluetoothEnabled) snprintf(buffer, sizeof(buffer), "Packets: %lu", bluetoothPackets++);
      else if (selectedHardware == 3 && loraEnabled) snprintf(buffer, sizeof(buffer), "Packets: %lu", loraPackets++);
      else snprintf(buffer, sizeof(buffer), "Disabled");
      break;
    case 2: // Logger
      snprintf(buffer, sizeof(buffer), "Enabled: %s", (selectedHardware == 0 ? sdEnabled : w25q128Enabled) ? "Yes" : "No");
      break;
    case 3: // Actuation
      if (selectedHardware == 0 && relay0Enabled) {
        snprintf(buffer, sizeof(buffer), "State: %s", actuators[0].state ? "On" : "Off");
      } else if (selectedHardware == 1 && pwm1Enabled) {
        snprintf(buffer, sizeof(buffer), "PWM: %u", actuators[1].pwmValue);
      } else {
        snprintf(buffer, sizeof(buffer), "Disabled");
      }
      break;
    case 4: // HID
      if (selectedHardware == 1) snprintf(buffer, sizeof(buffer), "Pot: %d", analogRead(POT_PIN));
      else snprintf(buffer, sizeof(buffer), "N/A");
      break;
    case 5: // FlightController
      snprintf(buffer, sizeof(buffer), "State: N/A");
      break;
    case 6: // Config
      snprintf(buffer, sizeof(buffer), "Screen Timeout: %lu s", screensaverTimeout / 1000);
      break;
    default:
      snprintf(buffer, sizeof(buffer), "No Data");
  }

  textWidth = strlen(buffer) * 6;
  if (textWidth > SCREEN_WIDTH - 10) {
    if (millis() - lastScrollTime > scrollDelay) {
      scrollOffset += 6; // Scroll by one character width
      if (scrollOffset > textWidth - (SCREEN_WIDTH - 10)) scrollOffset = 0;
      lastScrollTime = millis();
    }
    display.setCursor(10 - scrollOffset, 20);
  } else {
    scrollOffset = 0;
    display.setCursor(10, 20);
  }
  display.print(buffer);

  display.display();
}

void drawStatusIcons(bool bluetooth, bool rs485, bool canbus, bool lora) {
  int iconX = SCREEN_WIDTH - 40;
  int iconY = 2;
  int iconSpacing = 10;

  if (bluetooth) {
    display.drawBitmap(iconX, iconY, bluetoothIcon, 8, 8, SSD1306_WHITE);
    iconX += iconSpacing;
  }
  if (rs485) {
    display.drawBitmap(iconX, iconY, rs485Icon, 8, 8, SSD1306_WHITE);
    iconX += iconSpacing;
  }
  if (canbus) {
    display.drawBitmap(iconX, iconY, canbusIcon, 8, 8, SSD1306_WHITE);
    iconX += iconSpacing;
  }
  if (lora) {
    display.drawBitmap(iconX, iconY, loraIcon, 8, 8, SSD1306_WHITE);
  }
}

static void bootAnimation() {
  int leds[] = {LED_1_PIN, LED_2_PIN, LED_3_PIN, LED_4_PIN};

  for (int i = 0; i < 4; i++) {
    digitalWrite(leds[i], HIGH);
    delay(ANIMATION_DELAY);
  }

  delay(300);

  for (int i = 0; i < 4; i++) {
    digitalWrite(leds[i], LOW);
  }
}

static void saveSettingsToSD() {
#if ENABLE_SD
  FsFile settingsFile;
  if (settingsFile.open("settings.txt", O_RDWR | O_CREAT | O_TRUNC)) {
    settingsFile.print("MAX31855="); settingsFile.println(max31855Enabled ? "1" : "0");
    settingsFile.print("MPU6500="); settingsFile.println(mpu6500Enabled ? "1" : "0");
    settingsFile.print("ADS1115="); settingsFile.println(ads1115Enabled ? "1" : "0");
    settingsFile.print("RS485="); settingsFile.println(rs485Enabled ? "1" : "0");
    settingsFile.print("CANBUS="); settingsFile.println(canbusEnabled ? "1" : "0");
    settingsFile.print("Bluetooth="); settingsFile.println(bluetoothEnabled ? "1" : "0");
    settingsFile.print("LoRa="); settingsFile.println(loraEnabled ? "1" : "0");
    settingsFile.print("SD="); settingsFile.println(sdEnabled ? "1" : "0");
    settingsFile.print("W25Q128="); settingsFile.println(w25q128Enabled ? "1" : "0");
    settingsFile.print("Relay0="); settingsFile.println(relay0Enabled ? "1" : "0");
    settingsFile.print("PWM1="); settingsFile.println(pwm1Enabled ? "1" : "0");
    settingsFile.close();
  }
#endif
}

static void loadSettingsFromSD() {
#if ENABLE_SD
  FsFile settingsFile;
  if (settingsFile.open("settings.txt", O_READ)) {
    char line[32];
    while (settingsFile.available()) {
      settingsFile.fgets(line, sizeof(line));
      if (strncmp(line, "MAX31855=", 9) == 0) max31855Enabled = (line[9] == '1');
      else if (strncmp(line, "MPU6500=", 8) == 0) mpu6500Enabled = (line[8] == '1');
      else if (strncmp(line, "ADS1115=", 8) == 0) ads1115Enabled = (line[8] == '1');
      else if (strncmp(line, "RS485=", 6) == 0) rs485Enabled = (line[6] == '1');
      else if (strncmp(line, "CANBUS=", 7) == 0) canbusEnabled = (line[7] == '1');
      else if (strncmp(line, "Bluetooth=", 10) == 0) bluetoothEnabled = (line[10] == '1');
      else if (strncmp(line, "LoRa=", 5) == 0) loraEnabled = (line[5] == '1');
      else if (strncmp(line, "SD=", 3) == 0) sdEnabled = (line[3] == '1');
      else if (strncmp(line, "W25Q128=", 8) == 0) w25q128Enabled = (line[8] == '1');
      else if (strncmp(line, "Relay0=", 7) == 0) relay0Enabled = (line[7] == '1');
      else if (strncmp(line, "PWM1=", 5) == 0) pwm1Enabled = (line[5] == '1');
    }
    settingsFile.close();
  }
#endif
}

#endif