#include <SPI.h>
#include <SD.h>

/*
  Christina Walker: safety checks and data logging functions
  Noah Schatz: light and sound control functions
*/

// const int chipSelect = 10;
// File logFile;

// ESP32 pin assignments (GPIO numbers)
const int greenPins[] = {16, 17, 18, 19, 21};   // Green LED PWM outputs (green pins 1-5, flicker by flickerMode)
const int whitePins[] = {22, 23, 25, 26, 27};   // White LED PWM outputs (rotate one at a time)
const int numGreenPins = 5;
const int numWhitePins = 5;
const int speakerPin = 32;
const int volumePin = 33;
const int triggerPin = 34;                      // Input-only pin, no internal pull-up/pull-down
const int soundSensorPin = 35;                  // Input-only ADC pin for the sound level sensor
const int minimumVolume = 100;
const int maximumVolume = 255;
const int minimumFrequency = 3000;
const int maximumFrequency = 10000;
const int minimumFlicker = 100;                 // Shortest green flicker duration (ms)
const int maximumFlicker = 800;                 // Longest green flicker duration (ms)
const int minimumPause = 20;                    // Shortest pause between green flickers (ms)
const int maximumPause = 200;                   // Longest pause between green flickers (ms)

// Green flicker patterns for modes 1-3: each row is one flash, listing green pin numbers 1-5 (0 = unused)
const int greenPatternSteps = 5;
const int greenPatterns[3][greenPatternSteps][3] = {
  {{1, 0, 0}, {3, 0, 0}, {5, 0, 0}, {2, 0, 0}, {4, 0, 0}},   // Mode 1: star, one pin at a time
  {{1, 2, 0}, {4, 3, 0}, {5, 1, 0}, {2, 3, 0}, {5, 4, 0}},   // Mode 2: two pins at a time
  {{1, 2, 3}, {3, 4, 5}, {5, 1, 2}, {2, 3, 4}, {4, 5, 1}}    // Mode 3: three pins at a time
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
int volumeLevel = minimumVolume;
int volumePattern = -1;
int volumeStep = 0;
int state = 0;
int whiteIndex = 0;
unsigned long lastTime1 = 0;
unsigned long lastTime2 = 0;
unsigned long lastTime3 = 0;
unsigned long volumePatternStart = 0;




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

    if (volumePattern == -1) {
      volumePattern = random(0, 10);
      volumeStep = 0;
      volumePatternStart = millis();
    }

    runWhite();
    volume();
    speaker();
  }

  setGreen(0);                                                          // Turn off all lights when trigger pin is LOW
  for (int i = 0; i < numWhitePins; i++) {
    analogWrite(whitePins[i], 0);
  }
  noTone(speakerPin);
  analogWrite(volumePin, 0);
  volumePattern = -1;
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
void speaker() {                                     // Control the speaker frequency and sweep
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

  unsigned long time = millis();
  int speakerFreq = map(volumeLevel, minimumVolume, maximumVolume,
                        minimumFrequency, maximumFrequency);        // Map volume PWM to frequency.

  if (time - lastTime3 >= sweep) {
    lastTime3 = time;
    tone(speakerPin, speakerFreq);
  }
}

void volume() {              // Cycle through volume patterns and adjust the volume PWM accordingly
  int patternVolume = 0;
  unsigned long patternDuration = 100;
  unsigned long time = millis();

  if (digitalRead(triggerPin) == LOW) {
    noTone(speakerPin);
    analogWrite(volumePin, 0);
    volumePattern = -1;
    return;
  }

  switch (volumePattern) {
    case 0:
      switch (volumeStep) {
        case 0: patternVolume = 255; patternDuration = 120; break;
        case 1: patternVolume = 40; patternDuration = 240; break;
        case 2: patternVolume = 160; patternDuration = 100; break;
      }
      break;

    case 1:
      switch (volumeStep) {
        case 0: patternVolume = 30; patternDuration = 200; break;
        case 1: patternVolume = 255; patternDuration = 80; break;
        case 2: patternVolume = 20; patternDuration = 220; break;
      }
      break;

    case 2:
      switch (volumeStep) {
        case 0: patternVolume = 80; patternDuration = 150; break;
        case 1: patternVolume = 180; patternDuration = 150; break;
        case 2: patternVolume = 255; patternDuration = 150; break;
      }
      break;

    case 3:
      switch (volumeStep) {
        case 0: patternVolume = 255; patternDuration = 60; break;
        case 1: patternVolume = 0; patternDuration = 60; break;
        case 2: patternVolume = 255; patternDuration = 60; break;
      }
      break;

    case 4:
      switch (volumeStep) {
        case 0: patternVolume = 20; patternDuration = 300; break;
        case 1: patternVolume = 120; patternDuration = 200; break;
        case 2: patternVolume = 240; patternDuration = 100; break;
      }
      break;

    case 5:
      switch (volumeStep) {
        case 0: patternVolume = 220; patternDuration = 100; break;
        case 1: patternVolume = 60; patternDuration = 100; break;
        case 2: patternVolume = 220; patternDuration = 100; break;
      }
      break;

    case 6:
      switch (volumeStep) {
        case 0: patternVolume = 255; patternDuration = 200; break;
        case 1: patternVolume = 100; patternDuration = 100; break;
        case 2: patternVolume = 30; patternDuration = 300; break;
      }
      break;

    case 7:
      switch (volumeStep) {
        case 0: patternVolume = 50; patternDuration = 80; break;
        case 1: patternVolume = 200; patternDuration = 80; break;
        case 2: patternVolume = 50; patternDuration = 80; break;
      }
      break;

    case 8:
      switch (volumeStep) {
        case 0: patternVolume = 150; patternDuration = 250; break;
        case 1: patternVolume = 0; patternDuration = 120; break;
        case 2: patternVolume = 255; patternDuration = 250; break;
      }
      break;

    case 9:
      switch (volumeStep) {
        case 0: patternVolume = 255; patternDuration = 50; break;
        case 1: patternVolume = 150; patternDuration = 50; break;
        case 2: patternVolume = 30; patternDuration = 250; break;
      }
      break;
  }

  volumeLevel = constrain(patternVolume, minimumVolume, maximumVolume);   // Ensure volume level stays within the defined range
  analogWrite(volumePin, volumeLevel);

  if (time - volumePatternStart >= patternDuration) {                     // Move to the next step in the volume pattern
    volumePatternStart = time;
    if (volumeStep == 2) {
      volumePattern = random(0, 10);
      volumeStep = 0;
    } else {
      volumeStep++;
    }
  }
}
