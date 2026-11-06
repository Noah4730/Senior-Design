
/*
  Noah Schatz
  Green LED strobe lighting routine
*/

//------------------This-section-is-from-example-code--------------------------------------//

/*
  Mega analogWrite() test

  This sketch fades LEDs up and down one at a time on digital pins 2 through 13.
  This sketch was written for the Arduino Mega, and will not work on other boards.

  The circuit:
  - LEDs attached from pins 2 through 13 to ground.

  created 8 Feb 2009
  by Tom Igoe

  This example code is in the public domain.

  https://docs.arduino.cc/built-in-examples/analog/AnalogWriteMega/
*/

// These constants won't change. They're used to give names to the pins used:
const int lowestPin = 2;
const int highestPin = 13;


void setup() {
  // set pins 2 through 13 as outputs:
  for (int thisPin = lowestPin; thisPin <= highestPin; thisPin++) {
    pinMode(thisPin, OUTPUT);
  }
  pinMode(A0, INPUT); // Trigger Pin
  pinMode(A1, INPUT); // Microphone Pin
}

//------------------From-here-on-is-mine----------------------------------------------------//

int thisPin = 999;
int duration = 0;
int brightness = 255;
int freq = 35;  // This value is found by dividing 1000 by double the frequency you want.
int rotSpeed = 28;  // This is how fast the green lights rotate.
int state = 0;
int startPin = 3;
unsigned long lastTime1 = 0;
unsigned long lastTime2 = 0;


void loop() {

  // Cycles pins 3-12 with a sliding window of 3 sequential pins, strobing at each position

  while(digitalRead(A0) == HIGH) {  // Waits for trigger pull

    unsigned long now = millis(); // Capture current time once per loop

    if (now - lastTime1 >= (unsigned long)rotSpeed) {
      lastTime1 = now; // Reset timer
      startPin++;
      if (startPin > 12) startPin = 3;  // wrap back to start
      if (state == 1) {                 // if the state is ON then the 
        for (int i = startPin-1; i < startPin + 2; i++) {
          if (i < 3) i = 12;
          int x = i;
          if (x > 12) x = x - 10;
          analogWrite(x, 0);
        }
        for (int i = startPin; i < startPin + 3; i++) {
          int x = i;
          if (x > 12) x = x - 10;
          analogWrite(x, 255);
        }
      }
    }

    if (now - lastTime2 >= freq) {
      lastTime2 = now; // Reset timer
      if (state == 0) {
        for (int i = startPin; i < startPin + 3; i++) {
          int x = i;
          if (x > 12) x = x - 10;
          analogWrite(x, 255);
          state = 1;
        }
      } else {
        for (int i = startPin; i < startPin + 3; i++) {
          int x = i;
          if (x > 12) x = x - 10;
          analogWrite(x, 0);
          state = 0;
        }
      }
    }
  }
}
