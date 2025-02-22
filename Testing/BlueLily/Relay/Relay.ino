#define LED_RED 23
#define LED_YELLOW 22
#define LED_GREEN 37
#define LED_BLUE 36
#define TRIMPOT_PIN 17
#define EXTRA_PIN 21  // Pin to turn ON at maximum potentiometer position

const int ANIMATION_DELAY = 200; // Boot-up animation speed
const unsigned long IDLE_TIMEOUT = 500; // Idle timeout in milliseconds
const int TRIMPOT_DEADZONE = 25; // Ignore minor fluctuations
const int MAX_THRESHOLD = 1020;  // Potentiometer threshold to activate EXTRA_PIN

unsigned long lastInteractionTime = 0;
int previousLEDState = -1; // Track previous LED count
int previousTrimpotValue = -1; // Track previous trimpot reading

void setup() {
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(EXTRA_PIN, OUTPUT);
  pinMode(TRIMPOT_PIN, INPUT);

  Serial.begin(115200); // Debugging output

  bootAnimation();
}

void loop() {
  int trimpotValue = analogRead(TRIMPOT_PIN);
  int trimpotPercentage = map(trimpotValue, 0, 1023, 0, 100);

  int ledCount = getLEDCount(trimpotPercentage);

  // Update LEDs if trimpot value changes beyond the dead zone
  if (abs(trimpotValue - previousTrimpotValue) > TRIMPOT_DEADZONE) {
    updateLEDs(ledCount);
    previousLEDState = ledCount;
    previousTrimpotValue = trimpotValue;
    lastInteractionTime = millis(); // Reset idle timer on change
  }

  // Turn off LEDs if idle
  if (millis() - lastInteractionTime > IDLE_TIMEOUT) {
    turnOffLEDs();
    previousLEDState = -1; // Reset state
  }

  // Activate EXTRA_PIN when trimpot is near max value
  if (trimpotValue >= MAX_THRESHOLD) {
    digitalWrite(EXTRA_PIN, HIGH);
  } else {
    digitalWrite(EXTRA_PIN, LOW);
  }
}

void bootAnimation() {
  int leds[] = {LED_RED, LED_YELLOW, LED_GREEN, LED_BLUE};

  for (int i = 0; i < 4; i++) {
    digitalWrite(leds[i], HIGH);
    delay(ANIMATION_DELAY);
  }

  delay(300);

  for (int i = 0; i < 4; i++) {
    digitalWrite(leds[i], LOW);
  }
}

int getLEDCount(int percentage) {
  if (percentage >= 75) return 4;
  if (percentage >= 50) return 3;
  if (percentage >= 25) return 2;
  if (percentage > 5) return 1;
  return 0;
}

void updateLEDs(int count) {
  digitalWrite(LED_RED, count >= 1 ? HIGH : LOW);
  digitalWrite(LED_YELLOW, count >= 2 ? HIGH : LOW);
  digitalWrite(LED_GREEN, count >= 3 ? HIGH : LOW);
  digitalWrite(LED_BLUE, count >= 4 ? HIGH : LOW);
}

void turnOffLEDs() {
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_BLUE, LOW);
  digitalWrite(EXTRA_PIN, LOW);
}
