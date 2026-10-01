#include <SPI.h>
#include <SD.h>

/*
  Christina Walker: safety checks and data logging functions
  Noah Schatz: light and sound control functions
*/

// const int chipSelect = 10;
// File logFile;

const int lowestPin = 2;
const int highestPin = 13;

int freqGreen = 35;
int freqWhite = 40;
int rotSpeed = 28;
int speakerFreq = 1000;
int sweep = 50;
int state = 0;
int startPin = 3;
unsigned long lastTime1 = 0;
unsigned long lastTime2 = 0;
unsigned long lastTime3 = 0;




// Christina Walker - safety/input setup and data-logging behavior
void setup() {                                                        // Initialize pins
  for (int thisPin = lowestPin; thisPin <= highestPin; thisPin++) {
    pinMode(thisPin, OUTPUT);
  }
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);

  Serial.begin(115200);
}


void logEvent();
void runGreen();
void nbDelay();
void flicker();


// Noah Schatz - main light and sound trigger loop
void loop() {                           // Main loop

  while (digitalRead(A0) == HIGH) {     // Trigger pin activates light routine

    switch (random(0, 10)) {                    // Case Values generated with AI

      case 0:
        flicker(2, 350, 255); nbDelay(150);
        flicker(2, 800, 255); nbDelay(80);
        flicker(2, 200, 255); nbDelay(200);
        break;

      case 1:
        flicker(2, 600, 255); nbDelay(50);
        flicker(2, 250, 255); nbDelay(180);
        flicker(2, 750, 255); nbDelay(120);
        break;

      case 2:
        flicker(2, 800, 255); nbDelay(30);
        flicker(2, 300, 255); nbDelay(200);
        flicker(2, 550, 255); nbDelay(90);
        break;

      case 3:
        flicker(2, 400, 255); nbDelay(100);
        flicker(2, 700, 255); nbDelay(60);
        flicker(2, 150, 255); nbDelay(190);
        break;

      case 4:
        flicker(2, 200, 255); nbDelay(170);
        flicker(2, 650, 255); nbDelay(40);
        flicker(2, 800, 255); nbDelay(130);
        break;

      case 5:
        flicker(2, 500, 255); nbDelay(80);
        flicker(2, 100, 255); nbDelay(200);
        flicker(2, 750, 255); nbDelay(50);
        break;

      case 6:
        flicker(2, 800, 255); nbDelay(20);
        flicker(2, 350, 255); nbDelay(160);
        flicker(2, 600, 255); nbDelay(110);
        break;

      case 7:
        flicker(2, 450, 255); nbDelay(200);
        flicker(2, 800, 255); nbDelay(70);
        flicker(2, 300, 255); nbDelay(140);
        break;

      case 8:
        flicker(2, 700, 255); nbDelay(110);
        flicker(2, 200, 255); nbDelay(190);
        flicker(2, 500, 255); nbDelay(60);
        break;

      case 9:
        flicker(2, 250, 255); nbDelay(30);
        flicker(2, 800, 255); nbDelay(150);
        flicker(2, 400, 255); nbDelay(200);
        break;
    }

    runGreen();
    speaker();
  }

  for (int thisPin = lowestPin; thisPin <= highestPin; thisPin++) {     // Turn off all pins when trigger pin is LOW
    analogWrite(thisPin, 0);
  }
  noTone(13);
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
    if (startPin > 12) startPin = 3;

    if (state == 1) {                                           // If green lights are on, turn off the previous set and turn on the next set
      for (int i = startPin - 1; i < startPin + 2; i++) {
        int x = i;
        if (x < 3) x = 12;
        if (x > 12) x -= 10;

        analogWrite(x, 0);
        // logEvent("GREEN", x, "OFF");
      }

      for (int i = startPin; i < startPin + 3; i++) {
        int x = i;
        if (x > 12) x -= 10;

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
        if (x > 12) x -= 10;

        analogWrite(x, 255);
        // logEvent("GREEN", x, "ON");
      }
      state = 1;

    } else {
      for (int i = startPin; i < startPin + 3; i++) {
        int x = i;
        if (x > 12) x -= 10;

        analogWrite(x, 0);
        // logEvent("GREEN", x, "OFF");
      }
      state = 0;
    }
  }
}

// Noah Schatz - non-blocking timing helper for light/sound sequencing
void nbDelay(unsigned long ms) {          // Non-blocking delay function that allows other functions to run during the delay
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


  if(digitalRead(A0) == LOW) return;

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

  // Noah Schatz - audio output behavior
  unsigned long time = millis();

  if(time - lastTime3 >= sweep) {
    lastTime3 = time;
    tone(13, speakerFreq);
    speakerFreq = speakerFreq + 100;
  }

  if(speakerFreq > 3000) {
    speakerFreq = 1000;
  }
}
