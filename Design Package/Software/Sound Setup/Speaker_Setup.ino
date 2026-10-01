#include <Arduino.h>

/*
  Noah Schatz
  Speaker setup and audio control routines
*/

const int speakerPin = 13;
const int volumePin = 12;
const int triggerPin = A0;
const int minimumVolume = 100;
const int maximumVolume = 255;
const int minimumFrequency = 3000;
const int maximumFrequency = 10000;

int sweep = 50;
int volumeLevel = minimumVolume;
int volumePattern = -1;
int volumeStep = 0;
unsigned long lastTime3 = 0;
unsigned long volumePatternStart = 0;

void setup() {                                                  // Configure pins and start the speaker controls
  pinMode(speakerPin, OUTPUT);
  pinMode(volumePin, OUTPUT);
  pinMode(triggerPin, INPUT);

  analogWrite(volumePin, volumeLevel);
  randomSeed(analogRead(A3));
}

void loop() {                                                   // Run random volume patterns while the trigger is active
  if (digitalRead(triggerPin) == HIGH) {
    if (volumePattern == -1) {
      volumePattern = random(0, 10);
      volumeStep = 0;
      volumePatternStart = millis();
    }

    volume();
    speaker();
  } else {
    noTone(speakerPin);
    analogWrite(volumePin, 0);
    volumePattern = -1;
  }
}

void speaker() {
  if (digitalRead(triggerPin) == LOW) return;

  unsigned long time = millis();

  int speakerFreq = map(volumeLevel, minimumVolume, maximumVolume,
                        minimumFrequency, maximumFrequency);               // Map volume PWM to frequency.

  if (time - lastTime3 >= sweep) {
    lastTime3 = time;
    tone(speakerPin, speakerFreq);
  }
}

void volume() {                                                            // Cycle through volume patterns and adjust the volume PWM accordingly
  int patternVolume = 0;
  unsigned long patternDuration = 100;
  unsigned long time = millis();

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
