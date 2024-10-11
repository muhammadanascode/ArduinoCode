const int trigPin1 = 32;        // Trigger pin for the first ultrasonic sensor
const int echoPin1 = 25;        // Echo pin for the first ultrasonic sensor
const int trigPin2 = 33;        // Trigger pin for the second ultrasonic sensor
const int echoPin2 = 26;        // Echo pin for the second ultrasonic sensor
const int greenLedPin = 5;      // Pin for the green traffic light
const int redLedPin = 15;       // Pin for the red traffic light

#define SOUND_VELOCITY 0.034     // Speed of sound used to calculate distance

long duration1, duration2;       // Durations for both sensors
float distanceCm1, distanceCm2;  // Distances for both sensors
float lastDistanceCm1 = -1;      // Last distance value for sensor 1
float lastDistanceCm2 = -1;      // Last distance value for sensor 2
unsigned long lastTime1 = 0;      // Last time for sensor 1
unsigned long lastTime2 = 0;      // Last time for sensor 2
unsigned long greenSignalStartTime = 0; // Start time for green signal
unsigned long redSignalStartTime = 0;    // Start time for red signal
bool signalOpen = false;          // To indicate if the green signal is active
bool redSignalActive = false;     // To indicate if the red signal is active

void setup() {
  Serial.begin(115200);           // Start the serial communication

  // Set up pins for the first ultrasonic sensor
  pinMode(trigPin1, OUTPUT);      // Set trigPin1 as an Output
  pinMode(echoPin1, INPUT);       // Set echoPin1 as an Input

  // Set up pins for the second ultrasonic sensor
  pinMode(trigPin2, OUTPUT);      // Set trigPin2 as an Output
  pinMode(echoPin2, INPUT);       // Set echoPin2 as an Input

  // Set up LED pins
  pinMode(greenLedPin, OUTPUT);   // Set the green LED as an Output
  pinMode(redLedPin, OUTPUT);      // Set the red LED as an Output

  // Initially, set both signals to off
  digitalWrite(greenLedPin, LOW); // Green signal is off
  digitalWrite(redLedPin, HIGH);  // Red signal is on
}

void loop() {
  // Reset the first ultrasonic sensor
  digitalWrite(trigPin1, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin1, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin1, LOW);
  
  // Read the echo from the first ultrasonic sensor
  duration1 = pulseIn(echoPin1, HIGH);
  distanceCm1 = duration1 * SOUND_VELOCITY / 2;

  // Reset the second ultrasonic sensor
  digitalWrite(trigPin2, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin2, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin2, LOW);
  
  // Read the echo from the second ultrasonic sensor
  duration2 = pulseIn(echoPin2, HIGH);
  distanceCm2 = duration2 * SOUND_VELOCITY / 2;

  // Print distances on the Serial Monitor
  Serial.print("Distance 1 (cm): ");
  Serial.println(distanceCm1);
  Serial.print("Distance 2 (cm): ");
  Serial.println(distanceCm2);

  // Check if both distances are constant (within 1 cm)
  if (abs(distanceCm1 - lastDistanceCm1) < 1.0 && abs(distanceCm2 - lastDistanceCm2) < 1.0) {
    // If both distances are constant for 8 seconds and green light is not open
    if (millis() - lastTime1 >= 8000 && !signalOpen) {
      Serial.println("Both distances constant for 8 seconds. Opening green signal for 15 seconds...");
      digitalWrite(greenLedPin, HIGH);  // Turn on the green light
      digitalWrite(redLedPin, LOW);     // Turn off the red light
      signalOpen = true;                // Mark the signal as open
      greenSignalStartTime = millis();  // Record the signal open time
    }
  } else {
    // Reset the timer for sensor 1 if the distance changes
    lastTime1 = millis();  
  }

  // Check if the green signal is open
  if (signalOpen && millis() - greenSignalStartTime >= 15000) {
    Serial.println("Closing green signal after 15 seconds.");
    digitalWrite(greenLedPin, LOW);    // Turn off the green light
    digitalWrite(redLedPin, HIGH);     // Turn on the red light
    signalOpen = false;                 // Reset the signal state
    lastTime1 = millis();               // Reset the time for the next cycle
  }

  // Scenario 2: If the first sensor is constant but the second sensor is fluctuating
  if (abs(distanceCm1 - lastDistanceCm1) < 1.0 && abs(distanceCm2 - lastDistanceCm2) >= 1.0) {
    if (!redSignalActive) {
      Serial.println("Red signal active due to distance instability on sensor 2.");
      digitalWrite(redLedPin, HIGH);    // Turn on the red light (activate red signal)
      redSignalActive = true;           // Set red signal as active
      redSignalStartTime = millis();    // Record when the red signal became active
      digitalWrite(greenLedPin, LOW);   // Ensure green light is off when red is on
    }
  }

  // If red signal is active for 30 seconds
  if (redSignalActive && millis() - redSignalStartTime >= 30000) {
    Serial.println("Closing red signal after 30 seconds.");
    digitalWrite(redLedPin, LOW);     // Turn off the red light
    redSignalActive = false;           // Reset red signal state
    digitalWrite(greenLedPin, HIGH);   // Turn on the green light for 15 seconds
    signalOpen = true;                 // Mark the signal as open
    greenSignalStartTime = millis();   // Record the time when the green signal is turned on
    lastTime1 = millis();              // Reset the time for the next cycle
    delay(15000);                      // Keep red signal off for 15 seconds
  }

  // Update the last distance values for both sensors
  lastDistanceCm1 = distanceCm1;
  lastDistanceCm2 = distanceCm2;

  delay(1000);  // Wait for a second before measuring again
}
