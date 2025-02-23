// ** ICONS: Define 8x8 pixel bitmaps **
const uint8_t bluetoothIcon[] PROGMEM = {
  0b00011000, 0b00100100, 0b01000100, 0b10001000,
  0b01000100, 0b00100100, 0b00011000, 0b00000000
};

const uint8_t rs485Icon[] PROGMEM = {
  0b00011000, 0b00111100, 0b01111110, 0b01100110,
  0b01100110, 0b01111110, 0b00111100, 0b00011000
};

const uint8_t canbusIcon[] PROGMEM = {
  0b00111100, 0b01000010, 0b10011001, 0b10111101,
  0b10111101, 0b10011001, 0b01000010, 0b00111100
};

const uint8_t loraIcon[] PROGMEM = {
  0b00001000, 0b00010100, 0b00100010, 0b01000001,
  0b01000001, 0b00100010, 0b00010100, 0b00001000
};

const uint8_t rocketBitmap[] PROGMEM = {
  0b00011000, 0b00111100, 0b01111110, 0b11111111,
  0b01111110, 0b00111100, 0b00011000, 0b00011000
};

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1  
#define SCREEN_ADDRESS 0x3C 

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Potentiometer
#define POT_PIN 17
int lastPotValue = 0;
unsigned long lastInteractionTime = 0;
const unsigned long screensaverTimeout = 30000; // 30 seconds
const int deadbandThreshold = 50; // Ignore small variations

// Buttons
#define BTN_SELECT 26
#define BTN_BACK 27

// Status Flags
bool bluetoothEnabled = true;
bool rs485Enabled = true;
bool canbusEnabled = false;
bool loraEnabled = true;

// Menu Items
const char* mainMenuItems[] = {"Start", "Settings", "Info", "Exit"};
const char* settingsMenu[] = {"Brightness", "Sound", "Back"};
const char* infoMenu[] = {"Version", "Credits", "Back"};

const int mainMenuSize = sizeof(mainMenuItems) / sizeof(mainMenuItems[0]);
const int settingsMenuSize = sizeof(settingsMenu) / sizeof(settingsMenu[0]);
const int infoMenuSize = sizeof(infoMenu) / sizeof(infoMenu[0]);

// State variables
int menuIndex = 0;
bool inSubMenu = false;
bool inScreensaver = false;
const char** currentMenu = mainMenuItems;
int currentMenuSize = mainMenuSize;

// Scrollbar properties
int scrollbarHeight = 20;
int scrollbarX = SCREEN_WIDTH - 5;
int scrollbarY = 10;

// Screensaver Animation
int rocketY = SCREEN_HEIGHT - 10;
const int rocketX = SCREEN_WIDTH / 2 - 4;

void setup() {
    pinMode(BTN_SELECT, INPUT_PULLUP);
    pinMode(BTN_BACK, INPUT_PULLUP);

    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
        Serial.println(F("SSD1306 allocation failed"));
        for (;;);
    }

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.display();
    lastInteractionTime = millis();
}

void loop() {
    if (inScreensaver) {
        drawScreensaver();
        checkForInteraction();
    } else {
        handleInput();
        drawMenu();
    }
}

void handleInput() {
    int potValue = analogRead(POT_PIN);
    int mappedIndex = map(potValue, 0, 1023, 0, currentMenuSize - 1);
    
    if (mappedIndex != menuIndex) {
        menuIndex = mappedIndex;
        lastInteractionTime = millis();
    }

    lastPotValue = potValue;

    if (digitalRead(BTN_SELECT) == LOW) {
        lastInteractionTime = millis();
        if (!inSubMenu) {
            if (menuIndex == 1) {
                currentMenu = settingsMenu;
                currentMenuSize = settingsMenuSize;
                menuIndex = 0;
                inSubMenu = true;
            } else if (menuIndex == 2) {
                currentMenu = infoMenu;
                currentMenuSize = infoMenuSize;
                menuIndex = 0;
                inSubMenu = true;
            } else if (menuIndex == 3) {
                Serial.println("Exiting...");
            }
        } else {
            if (menuIndex == currentMenuSize - 1) {
                currentMenu = mainMenuItems;
                currentMenuSize = mainMenuSize;
                menuIndex = 0;
                inSubMenu = false;
            }
        }
        delay(300);
    }

    if (digitalRead(BTN_BACK) == LOW && inSubMenu) {
        currentMenu = mainMenuItems;
        currentMenuSize = mainMenuSize;
        menuIndex = 0;
        inSubMenu = false;
        lastInteractionTime = millis();
        delay(300);
    }

    if (millis() - lastInteractionTime > screensaverTimeout) {
        inScreensaver = true;
    }
}

void checkForInteraction() {
    int potValue = analogRead(POT_PIN);
    
    if (abs(potValue - lastPotValue) > deadbandThreshold || 
        digitalRead(BTN_SELECT) == LOW || 
        digitalRead(BTN_BACK) == LOW) {
        inScreensaver = false;
        lastInteractionTime = millis();
        delay(300); // Debounce delay
    }
    
    lastPotValue = potValue;
}

void drawMenu() {
    display.clearDisplay();

    // Title Bar
    display.setTextSize(1);
    display.setCursor(10, 2);
    display.print(inSubMenu ? "Submenu" : "Main Menu");

    // Status Icons
    drawStatusIcons();

    // Menu Section
    for (int i = 0; i < currentMenuSize; i++) {
        int yPos = 20 + (i * 10);
        display.setCursor(10, yPos);
        if (i == menuIndex) {
            display.print("> ");
        } else {
            display.print("  ");
        }
        display.print(currentMenu[i]);
    }

    // Draw Scrollbar
    int scrollBarMaxY = SCREEN_HEIGHT - scrollbarHeight - 5;
    int scrollBarPosition = map(menuIndex, 0, currentMenuSize - 1, 10, scrollBarMaxY);
    display.fillRect(scrollbarX, scrollBarPosition, 3, scrollbarHeight, SSD1306_WHITE);

    display.display();
}

void drawScreensaver() {
    display.clearDisplay();

    drawStatusIcons();
    display.setCursor(0, 0);
    display.print("GodSpeed");
    display.setCursor(40, 30);
    display.print("Bluelily...");

    drawRocket(rocketX, rocketY);

    rocketY -= 1;
    if (rocketY < -10) {
        rocketY = SCREEN_HEIGHT - 10;
    }

    display.display();
    delay(50);
}

void drawStatusIcons() {
    int iconX = SCREEN_WIDTH - 40;
    int iconY = 2;
    int iconSpacing = 10;

    if (bluetoothEnabled) {
        display.drawBitmap(iconX, iconY, bluetoothIcon, 8, 8, SSD1306_WHITE);
        iconX += iconSpacing;
    }
    if (rs485Enabled) {
        display.drawBitmap(iconX, iconY, rs485Icon, 8, 8, SSD1306_WHITE);
        iconX += iconSpacing;
    }
    if (canbusEnabled) {
        display.drawBitmap(iconX, iconY, canbusIcon, 8, 8, SSD1306_WHITE);
        iconX += iconSpacing;
    }
    if (loraEnabled) {
        display.drawBitmap(iconX, iconY, loraIcon, 8, 8, SSD1306_WHITE);
    }
}

void drawRocket(int x, int y) {
    display.drawBitmap(x, y, rocketBitmap, 8, 8, SSD1306_WHITE);
}