#define LED_RED 23
#define LED_YELLOW 22
#define LED_GREEN 37
#define LED_BLUE 36
#define TRIMPOT_PIN 17

const int ANIMATION_DELAY = 200;    // Boot-up animation speed
const unsigned long IDLE_TIMEOUT = 500; // Idle timeout in milliseconds
const int TRIMPOT_DEADZONE = 30;    // Ignore minor fluctuations
const int FADE_DELAY = 1;           // Delay for fading animations
const int FADE_STEPS = 10;         // Steps for smooth fading (0-255 brightness)

unsigned long lastInteractionTime = 0;
int previousLEDState = -1;          // Track previous LED count
int previousTrimpotValue = -1;      // Track previous trimpot reading

void setup() {
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(TRIMPOT_PIN, INPUT);

  Serial.begin(115200); // Debugging output
  bootAnimation();
}

void loop() {
  int trimpotValue = analogRead(TRIMPOT_PIN);
  int trimpotPercentage = map(trimpotValue, 0, 1023, 0, 100);

  int ledCount = getLEDCount(trimpotPercentage);
  Serial.println(abs(trimpotValue - previousTrimpotValue));
  // Update LEDs if trimpot value changes beyond the dead zone
  if (abs(trimpotValue - previousTrimpotValue) > TRIMPOT_DEADZONE) {
    fadeInLEDs(ledCount, trimpotPercentage);
    previousLEDState = ledCount;
    previousTrimpotValue = trimpotValue;
    lastInteractionTime = millis(); // Reset idle timer on change
    Serial.println("change");
  }

  // Turn off LEDs if idle
  if (millis() - lastInteractionTime > IDLE_TIMEOUT) {
    fadeOutAllLEDs();
    previousLEDState = -1;
    Serial.println("off");
  }
}

void bootAnimation() {
  int leds[] = {LED_RED, LED_YELLOW, LED_GREEN, LED_BLUE};
  for (int i = 0; i < 4; i++) {
    fadeInSingleLED(leds[i], 100);
    delay(ANIMATION_DELAY);
    fadeOutSingleLED(leds[i]);
  }
}

int getLEDCount(int percentage) {
  if (percentage >= 75) return 4;
  if (percentage >= 50) return 3;
  if (percentage >= 25) return 2;
  if (percentage > 5) return 1;
  return 0;
}

void fadeInLEDs(int count, int trimpotPercentage) {
  int brightnessPerLED = trimpotPercentage; // Vary brightness from 0-100
  int leds[] = {LED_RED, LED_YELLOW, LED_GREEN, LED_BLUE};
  for (int i = 0; i < count; i++) {
    fadeInSingleLED(leds[i], brightnessPerLED);
  }
  for (int i = count; i < 4; i++) {
    fadeOutSingleLED(leds[i]);
  }
}

void fadeInSingleLED(int pin, int maxBrightness) {
  maxBrightness = map(maxBrightness, 0, 100, 0, 255);
  for (int brightness = 0; brightness <= maxBrightness; brightness++) {
    analogWrite(pin, brightness);
    delay(FADE_DELAY);
  }
}

void fadeOutSingleLED(int pin) {
  for (int brightness = analogRead(pin); brightness >= 0; brightness--) {
    analogWrite(pin, brightness);
    delay(FADE_DELAY);
  }
  analogWrite(pin, 0);
}

void fadeOutAllLEDs() {
  int leds[] = {LED_RED, LED_YELLOW, LED_GREEN, LED_BLUE};
  for (int i = 0; i < 4; i++) {
    fadeOutSingleLED(leds[i]);
  }
}
