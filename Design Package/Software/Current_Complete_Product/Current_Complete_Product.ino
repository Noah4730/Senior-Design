#include <SPI.h>
#include <SD.h>

/*
  Christina Walker: safety checks and data logging functions
  Noah Schatz: light and sound control functions
*/

// const int chipSelect = 10;
// File logFile;

const int greenPins[] = {16, 17, 18, 19, 21};
const int whitePins[] = {22, 23, 25, 26, 27};
const int numGreenPins = 5;
const int numWhitePins = 5;
const int speakerPin = 32;
const int volumePin = 33;
const int triggerPin = 34;
const int soundSensorPin = 35;
const int LVmin = 20;
const int LVmax = 80;
const int HVmin = 120;
const int HVmax = 255;
const int RMSout = 160;
const int minimumLVDuration = 250;
const int maximumLVDuration = 750;
const int minimumHVDuration = 500;
const int maximumHVDuration = 1500;
const int minimumFrequency = 3000;
const int maximumFrequency = 10000;
const int frequencyOffset = 3000;
const int minimumFlicker = 100;
const int maximumFlicker = 800;
const int minimumPause = 20;
const int maximumPause = 200;


const int greenPatternSteps = 5;
const int greenPatterns[3][greenPatternSteps][3] = {         // Green flicker patterns for modes 1-3
  {{1, 0, 0}, {3, 0, 0}, {5, 0, 0}, {2, 0, 0}, {4, 0, 0}},   // star, one pin at a time
  {{1, 2, 0}, {4, 3, 0}, {5, 1, 0}, {2, 3, 0}, {5, 4, 0}},   // two pins at a time
  {{1, 2, 3}, {3, 4, 5}, {5, 1, 2}, {2, 3, 4}, {4, 5, 1}}    // three pins at a time
};

int freqWhite = 35;
int freqGreen = 40;
int greenBrightness = 255;                      // Brightness for all green pins (0-255)
int whiteBrightness = 255;                      // Brightness for all white pins (0-255)
int flickerMode = 0;                            // 0 = all at once, 1 = star, 2 = two at a time, 3 = three at a time
int greenStep = 0;                              // Current flash in the green flicker pattern
int rotSpeed = 28;
int speakerFreq = 1000;
int sweep = 50;
int volumeLevel = LVmin;
int volumePhase = -1;                           // -1 = off, 0 = low volume, 1 = high volume
int state = 0;
int whiteIndex = 0;
unsigned long lastTime1 = 0;
unsigned long lastTime2 = 0;
unsigned long lastTime3 = 0;
unsigned long volumePhaseStart = 0;
unsigned long volumePhaseDuration = 0;
unsigned long outputTime = 0;                   // Total ms counted in the running RMS
float outputSquareSum = 0;                      // Sum of volumeLevel^2 * ms counted in the running RMS




// Christina Walker - safety/input setup and data-logging behavior
void setup() {                                                        // Initialize pins
  for (int i = 0; i < numGreenPins; i++) {
    pinMode(greenPins[i], OUTPUT);
  }
  for (int i = 0; i < numWhitePins; i++) {
    pinMode(whitePins[i], OUTPUT);
  }
  pinMode(speakerPin, OUTPUT);
  pinMode(volumePin, OUTPUT);
  pinMode(triggerPin, INPUT);
  pinMode(soundSensorPin, INPUT);

  analogWrite(volumePin, volumeLevel);
  Serial.begin(115200);
}


void logEvent();
void runWhite();
void sysTime();
void flicker();


// Noah Schatz - main light and sound trigger loop
void loop() {                           // Main loop with green light algorithm and major function calls

  while (digitalRead(triggerPin) == HIGH) {     // Trigger pin activates light routine

    flicker(random(minimumFlicker, maximumFlicker + 1));     // Flicker green for a random duration
    sysTime(random(minimumPause, maximumPause + 1));         // Pause green for a random duration

    runWhite();
    speaker();
  }

  setGreen(0);                                                          // Turn off all lights when trigger pin is LOW
  for (int i = 0; i < numWhitePins; i++) {
    analogWrite(whitePins[i], 0);
  }
  noTone(speakerPin);
  analogWrite(volumePin, 0);
  volumePhase = -1;
  state = 0;
  greenStep = 0;

  // Christina Walker - safety/data logging readout
  int sensorValue = analogRead(soundSensorPin); //
  float dB = 30 + (sensorValue / 4095.0) * 104; //sensorValue/4095.0 (ESP32 12-bit ADC): divides the raw reading by the max possible value, giving number between 0 and 1 (percentage)
                                                //x104: scales up to a range of 0 to 104 (134-30)
                                                //+30: shifts the whole thing up so the minimum is 30 instead of 0

  // Christina Walker - serial logging output for safety monitoring
  Serial.print(millis());
  Serial.print(", ");
  Serial.print(sensorValue);
  Serial.print(", ");
  Serial.println(dB);

}

// Christina Walker - safety and logging helper
void logEvent(const char* type, int pin, const char* stateStr) {      // Log events to serial monitor

  if(digitalRead(triggerPin) == LOW) return;

  Serial.print(millis());
  Serial.print(",");
  Serial.print(type);
  Serial.print(",");
  Serial.print(pin);
  Serial.print(",");
  Serial.println(stateStr);
}

// Noah Schatz - white LED light routine
void runWhite() {                                     // Run white light routine
  unsigned long now = millis();

  if(digitalRead(triggerPin) == LOW) return;                  // If trigger pin is LOW, exit function

  if (now - lastTime1 >= (unsigned long)rotSpeed) {             // Rotate the white lights every rotSpeed milliseconds
    lastTime1 = now;
    analogWrite(whitePins[whiteIndex], 0);                      // Turn off the current pin before moving to the next one
    // logEvent("WHITE", whitePins[whiteIndex], "OFF");

    whiteIndex++;
    if (whiteIndex >= numWhitePins) whiteIndex = 0;

    if (state == 1) {                                           // If white lights are on, turn on only the next pin
      analogWrite(whitePins[whiteIndex], whiteBrightness);
      // logEvent("WHITE", whitePins[whiteIndex], "ON");
    }
  }

  if (now - lastTime2 >= (unsigned long)freqWhite) {       // Toggle the active white light on and off every freqWhite milliseconds
    lastTime2 = now;

    if (state == 0) {
      analogWrite(whitePins[whiteIndex], whiteBrightness);
      // logEvent("WHITE", whitePins[whiteIndex], "ON");
      state = 1;

    } else {
      analogWrite(whitePins[whiteIndex], 0);
      // logEvent("WHITE", whitePins[whiteIndex], "OFF");
      state = 0;
    }
  }
}

// Noah Schatz - non-blocking timing helper for light/sound sequencing
void sysTime(unsigned long ms) {          // Delay function based on system time that allows semi parallel functions
  if(digitalRead(triggerPin) == LOW) return;

  unsigned long t = millis();
  while (millis() - t < ms) {
    if(digitalRead(triggerPin) == LOW) return;
    runWhite();
    speaker();
  }
}

// Noah Schatz - set every green LED pin to the same brightness
void setGreen(int brightness) {
  for (int i = 0; i < numGreenPins; i++) {
    analogWrite(greenPins[i], brightness);
  }
}

// Noah Schatz - turn on the green pins for the current flash of the selected flicker mode
void showGreenStep() {
  if (flickerMode < 1 || flickerMode > 3) {               // Mode 0 (or invalid): all pins at once
    setGreen(greenBrightness);
    return;
  }

  for (int j = 0; j < 3; j++) {
    int pinNumber = greenPatterns[flickerMode - 1][greenStep][j];
    if (pinNumber > 0) analogWrite(greenPins[pinNumber - 1], greenBrightness);
  }

  greenStep++;                                             // Advance to the next flash in the pattern
  if (greenStep >= greenPatternSteps) greenStep = 0;
}

// Noah Schatz - green LED strobe/flicker routine
void flicker(int duration) {  // Flicker green pins for a certain duration at greenBrightness using flickerMode

  if(digitalRead(triggerPin) == LOW) return;

  for (int i = 0; i < (duration / 70); i++) {

    if(digitalRead(triggerPin) == LOW) return;

    showGreenStep();
    // logEvent("GREEN", greenPins[0], "ON");

    unsigned long t = millis();
    while (millis() - t < (unsigned long)freqGreen) {
      if(digitalRead(triggerPin) == LOW) { setGreen(0); return; }
      runWhite();
      speaker();
    }

    setGreen(0);
    // logEvent("GREEN", greenPins[0], "OFF");

    t = millis();
    while (millis() - t < (unsigned long)freqGreen) {
      if(digitalRead(triggerPin) == LOW) return;
      runWhite();
      speaker();
    }
  }
}

// Noah Schatz - sound control routine
void speaker() {                                     // Control the speaker volume, frequency, and sweep
  if (digitalRead(triggerPin) == LOW) return;

  // Christina Walker - data logging during sound activation
  int sensorValue = analogRead(soundSensorPin); //
  float dB = 30 + (sensorValue / 4095.0) * 104; //sensorValue/4095.0 (ESP32 12-bit ADC): divides the raw reading by the max possible value, giving number between 0 and 1 (percentage)
                                                //x104: scales up to a range of 0 to 104 (134-30)
                                                //+30: shifts the whole thing up so the minimum is 30 instead of 0

  // Christina Walker - serial output for sound-trigger logging
  Serial.print(millis());
  Serial.print(", ");
  Serial.print(sensorValue);
  Serial.print(", ");
  Serial.println(dB);

  volume();

  unsigned long time = millis();

  if (time - lastTime3 >= sweep) {                                     // Pick a new random frequency every sweep milliseconds
    lastTime3 = time;
    int mappedFreq = map(volumeLevel, LVmin, HVmax, minimumFrequency, maximumFrequency);   // Map volume PWM to frequency
    mappedFreq = constrain(mappedFreq, minimumFrequency, maximumFrequency);
    int lowFreq = max(mappedFreq - frequencyOffset, minimumFrequency);    // Keep the random offset inside the frequency range
    int highFreq = min(mappedFreq + frequencyOffset, maximumFrequency);
    speakerFreq = random(lowFreq, highFreq + 1);
    tone(speakerPin, speakerFreq);
  }
}

void volume() {              // Alternate random low and high volume phases, steering the running RMS toward RMSout
  unsigned long time = millis();

  if (volumePhase == -1) {
    outputSquareSum = 0;
    outputTime = 0;
  } else {
    if (time - volumePhaseStart < volumePhaseDuration) return;           // if the current volume phase is still running

    outputSquareSum += (float)volumeLevel * volumeLevel * volumePhaseDuration;   // Add the finished phase to the running RMS
    outputTime += volumePhaseDuration;
  }

  if (volumePhase == 1) {                                                 // After a high phase, play a random low volume phase
    volumePhase = 0;
    volumeLevel = random(LVmin, LVmax + 1);
    volumePhaseDuration = random(minimumLVDuration, maximumLVDuration + 1);

  } else {                                                                // After a low phase (or a new trigger press), play a random high volume phase
    volumePhase = 1;
    float currentRMS = 0;
    if (outputTime > 0) currentRMS = sqrt(outputSquareSum / outputTime);

    int middleHV = (HVmin + HVmax) / 2;
    if (currentRMS > RMSout) {                                            // Above RMSout: use the quieter half of the high range
      volumeLevel = random(HVmin, middleHV + 1);
    } else {                                                              // At or below RMSout: use the louder half of the high range
      volumeLevel = random(middleHV, HVmax + 1);
    }
    volumePhaseDuration = random(minimumHVDuration, maximumHVDuration + 1);
  }

  volumePhaseStart = time;
  analogWrite(volumePin, volumeLevel);
}
