// ** ICONS **
const uint8_t bluetoothIcon[] PROGMEM = {
  0b00011000,
  0b00100100,
  0b01000100,
  0b10001000,
  0b01000100,
  0b00100100,
  0b00011000,
  0b00000000
};

const uint8_t rs485Icon[] PROGMEM = {
  0b00011000,
  0b00111100,
  0b01111110,
  0b01100110,
  0b01100110,
  0b01111110,
  0b00111100,
  0b00011000
};

const uint8_t canbusIcon[] PROGMEM = {
  0b00111100,
  0b01000010,
  0b10011001,
  0b10111101,
  0b10111101,
  0b10011001,
  0b01000010,
  0b00111100
};

const uint8_t loraIcon[] PROGMEM = {
  0b00001000,
  0b00010100,
  0b00100010,
  0b01000001,
  0b01000001,
  0b00100010,
  0b00010100,
  0b00001000
};

const uint8_t rocketBitmap[] PROGMEM = {
  0b00011000,
  0b00111100,
  0b01111110,
  0b11111111,
  0b01111110,
  0b00111100,
  0b00011000,
  0b00011000
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
const unsigned long screensaverTimeout = 3000; // 10 seconds
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
const int mainMenuSize = sizeof(mainMenuItems) / sizeof(mainMenuItems[0]);

int menuIndex = 0;
bool inScreensaver = false;

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
    int mappedIndex = map(potValue, 0, 1023, 0, mainMenuSize - 1);
    
    if (mappedIndex != menuIndex) {
        menuIndex = mappedIndex;
        lastInteractionTime = millis();
    }

    if (digitalRead(BTN_SELECT) == LOW) {
        lastInteractionTime = millis();
        // Perform menu selection
        delay(300);
    }

    if (millis() - lastInteractionTime > screensaverTimeout) {
        inScreensaver = true;
    }
}

void checkForInteraction() {
    int potValue = analogRead(POT_PIN);
    
    // Only consider movement if it exceeds the deadband threshold
    if (abs(potValue - lastPotValue) > deadbandThreshold || digitalRead(BTN_SELECT) == LOW) {
        inScreensaver = false;
        lastInteractionTime = millis();
        delay(300); // Debounce delay
    }
    
    lastPotValue = potValue;
}

void drawMenu() {
    display.clearDisplay();

    display.setCursor(10, 2);
    display.print("Main Menu");

    drawStatusIcons();

    for (int i = 0; i < mainMenuSize; i++) {
        int yPos = 20 + (i * 10);
        display.setCursor(10, yPos);
        display.print(i == menuIndex ? "> " : "  ");
        display.print(mainMenuItems[i]);
    }

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

