#include <SPI.h>
#include <SD.h>

/*
  Christina Walker: safety checks and data logging functions
  Noah Schatz: light and sound control functions
*/

// const int chipSelect = 10;
// File logFile;

const int lowestPin = 3;
const int highestPin = 11;
const int speakerPin = 13;
const int volumePin = 12;
const int triggerPin = A0;
const int minimumVolume = 100;
const int maximumVolume = 255;
const int minimumFrequency = 3000;
const int maximumFrequency = 10000;

int freqGreen = 35;
int freqWhite = 40;
int rotSpeed = 28;
int speakerFreq = 1000;
int sweep = 50;
int volumeLevel = minimumVolume;
int volumePattern = -1;
int volumeStep = 0;
int state = 0;
int startPin = 3;
unsigned long lastTime1 = 0;
unsigned long lastTime2 = 0;
unsigned long lastTime3 = 0;
unsigned long volumePatternStart = 0;




// Christina Walker - safety/input setup and data-logging behavior
void setup() {                                                        // Initialize pins
  for (int thisPin = lowestPin; thisPin <= highestPin; thisPin++) {
    pinMode(thisPin, OUTPUT);
  }
  pinMode(speakerPin, OUTPUT);
  pinMode(volumePin, OUTPUT);
  pinMode(triggerPin, INPUT);
  pinMode(A1, INPUT);

  analogWrite(volumePin, volumeLevel);
  randomSeed(analogRead(A3));
  Serial.begin(115200);
}


void logEvent();
void runGreen();
void sysTime();
void flicker();


// Noah Schatz - main light and sound trigger loop
void loop() {                           // Main loop

  while (digitalRead(A0) == HIGH) {     // Trigger pin activates light routine

    switch (random(0, 10)) {                    // Case Values generated with AI

      case 0:
        flicker(2, 350, 255); sysTime(150);
        flicker(2, 800, 255); sysTime(80);
        flicker(2, 200, 255); sysTime(200);
        break;

      case 1:
        flicker(2, 600, 255); sysTime(50);
        flicker(2, 250, 255); sysTime(180);
        flicker(2, 750, 255); sysTime(120);
        break;

      case 2:
        flicker(2, 800, 255); sysTime(30);
        flicker(2, 300, 255); sysTime(200);
        flicker(2, 550, 255); sysTime(90);
        break;

      case 3:
        flicker(2, 400, 255); sysTime(100);
        flicker(2, 700, 255); sysTime(60);
        flicker(2, 150, 255); sysTime(190);
        break;

      case 4:
        flicker(2, 200, 255); sysTime(170);
        flicker(2, 650, 255); sysTime(40);
        flicker(2, 800, 255); sysTime(130);
        break;

      case 5:
        flicker(2, 500, 255); sysTime(80);
        flicker(2, 100, 255); sysTime(200);
        flicker(2, 750, 255); sysTime(50);
        break;

      case 6:
        flicker(2, 800, 255); sysTime(20);
        flicker(2, 350, 255); sysTime(160);
        flicker(2, 600, 255); sysTime(110);
        break;

      case 7:
        flicker(2, 450, 255); sysTime(200);
        flicker(2, 800, 255); sysTime(70);
        flicker(2, 300, 255); sysTime(140);
        break;

      case 8:
        flicker(2, 700, 255); sysTime(110);
        flicker(2, 200, 255); sysTime(190);
        flicker(2, 500, 255); sysTime(60);
        break;

      case 9:
        flicker(2, 250, 255); sysTime(30);
        flicker(2, 800, 255); sysTime(150);
        flicker(2, 400, 255); sysTime(200);
        break;
    }

    if (volumePattern == -1) {
      volumePattern = random(0, 10);
      volumeStep = 0;
      volumePatternStart = millis();
    }

    runGreen();
    volume();
    speaker();
  }

  for (int thisPin = lowestPin; thisPin <= highestPin; thisPin++) {     // Turn off all pins when trigger pin is LOW
    analogWrite(thisPin, 0);
  }
  noTone(speakerPin);
  analogWrite(volumePin, 0);
  volumePattern = -1;
  state = 0;

  // Christina Walker - safety/data logging readout
  int sensorValue = analogRead(A1); //
  float dB = 30 + (sensorValue / 1023.0) * 104; //sensorValue/1023.0: divides the raw reading by the max possible value, giving number between 0 and 1 (percentage)
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

  if(digitalRead(A0) == LOW) return;

  Serial.print(millis());
  Serial.print(",");
  Serial.print(type);
  Serial.print(",");
  Serial.print(pin);
  Serial.print(",");
  Serial.println(stateStr);
}

// Noah Schatz - green LED light routine
void runGreen() {                                     // Run green light routine
  unsigned long now = millis();

  if(digitalRead(A0) == LOW) return;                  // If trigger pin is LOW, exit function

  if (now - lastTime1 >= (unsigned long)rotSpeed) {             // Rotate the green lights every rotSpeed milliseconds
    lastTime1 = now;
    startPin++;
    if (startPin > 11) startPin = 3;

    if (state == 1) {                                           // If green lights are on, turn off the previous set and turn on the next set
      for (int i = startPin - 1; i < startPin + 2; i++) {
        int x = i;
        if (x < 3) x = 11;
        if (x > 11) x = 3;

        analogWrite(x, 0);
        // logEvent("GREEN", x, "OFF");
      }

      for (int i = startPin; i < startPin + 3; i++) {
        int x = i;
        if (x > 11) x = 3;

        analogWrite(x, 255); 
        // logEvent("GREEN", x, "ON");
      }
    }
  }

  if (now - lastTime2 >= (unsigned long)freqGreen) {            // Toggle the green lights on and off every freqGreen milliseconds
    lastTime2 = now;

    if (state == 0) {
      for (int i = startPin; i < startPin + 3; i++) {
        int x = i;
        if (x > 11) x = 3;

        analogWrite(x, 255);
        // logEvent("GREEN", x, "ON");
      }
      state = 1;

    } else {
      for (int i = startPin; i < startPin + 3; i++) {
        int x = i;
        if (x > 11) x = 3;

        analogWrite(x, 0);
        // logEvent("GREEN", x, "OFF");
      }
      state = 0;
    }
  }
}

// Noah Schatz - non-blocking timing helper for light/sound sequencing
void sysTime(unsigned long ms) {          // Non-blocking delay function that allows other functions to run during the delay
  if(digitalRead(A0) == LOW) return;

  unsigned long t = millis();
  while (millis() - t < ms) {
    if(digitalRead(A0) == LOW) return;
    runGreen();
    speaker();
  }
}

// Noah Schatz - white LED strobe/flicker routine
void flicker(int thisPin, int duration, int brightness) {       // Flicker a specific pin for a certain duration and brightness

  if(digitalRead(A0) == LOW) return;
  
  for (int i = 0; i < (duration / 70); i++) {

    if(digitalRead(A0) == LOW) return;

    analogWrite(thisPin, brightness);
    // logEvent("WHITE", thisPin, "ON");

    unsigned long t = millis();
    while (millis() - t < (unsigned long)freqWhite) {
      if(digitalRead(A0) == LOW) { analogWrite(thisPin, 0); return; }
      runGreen();
      speaker();
    }

    analogWrite(thisPin, 0);
    // logEvent("WHITE", thisPin, "OFF");

    t = millis();
    while (millis() - t < (unsigned long)freqWhite) {
      if(digitalRead(A0) == LOW) return;
      runGreen();
      speaker();
    }
  }
}

// Noah Schatz - sound control routine
void speaker() {                                          // Control the speaker frequency and sweep
  if (digitalRead(triggerPin) == LOW) return;

  // Christina Walker - data logging during sound activation
  int sensorValue = analogRead(A1); //
  float dB = 30 + (sensorValue / 1023.0) * 104; //sensorValue/1023.0: divides the raw reading by the max possible value, giving number between 0 and 1 (percentage)
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
