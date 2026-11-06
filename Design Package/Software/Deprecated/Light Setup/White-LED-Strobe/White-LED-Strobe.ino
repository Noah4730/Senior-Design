
/*
  Noah Schatz
  White LED strobe lighting routine
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
float freq = 40;  // This value is found by dividing 1000 by double the frequency you want.

void flicker(int thisPin, int duration, int brightness);

void loop() {

  // This section is for the white LEDs which are all connected on input 2

  while(digitalRead(A0) == HIGH) {  // Waits for trigger pull

    switch (random(0, 9)) {       // Randomly cycles through a series of disorienting strobe combinations.

      case 0:
        flicker(2, 350, 255);
        delay(150);
        flicker(2, 800, 255);
        delay(80);
        flicker(2, 200, 255);
        delay(200);
        break;

      case 1:
        flicker(2, 600, 255);
        delay(50);
        flicker(2, 250, 255);
        delay(180);
        flicker(2, 750, 255);
        delay(120);
        break;

      case 2:
        flicker(2, 800, 255);
        delay(30);
        flicker(2, 300, 255);
        delay(200);
        flicker(2, 550, 255);
        delay(90);
        break;

      case 3:
        flicker(2, 400, 255);
        delay(100);
        flicker(2, 700, 255);
        delay(60);
        flicker(2, 150, 255);
        delay(190);
        break;

      case 4:
        flicker(2, 200, 255);
        delay(170);
        flicker(2, 650, 255);
        delay(40);
        flicker(2, 800, 255);
        delay(130);
        break;

      case 5:
        flicker(2, 500, 255);
        delay(80);
        flicker(2, 100, 255);
        delay(200);
        flicker(2, 750, 255);
        delay(50);
        break;

      case 6:
        flicker(2, 800, 255);
        delay(20);
        flicker(2, 350, 255);
        delay(160);
        flicker(2, 600, 255);
        delay(110);
        break;

      case 7:
        flicker(2, 450, 255);
        delay(200);
        flicker(2, 800, 255);
        delay(70);
        flicker(2, 300, 255);
        delay(140);
        break;

      case 8:
        flicker(2, 700, 255);
        delay(110);
        flicker(2, 200, 255);
        delay(190);
        flicker(2, 500, 255);
        delay(60);
        break;

      case 9:
        flicker(2, 250, 255);
        delay(30);
        flicker(2, 800, 255);
        delay(150);
        flicker(2, 400, 255);
        delay(200);
        break;

    }
  }
}

void flicker(int thisPin, int duration, int brightness) {

    for (int i = 0; i < (duration/70); i++) {
      analogWrite(thisPin, brightness);
      delay(freq);
      analogWrite(thisPin, 0);
      delay(freq);
    }

}
