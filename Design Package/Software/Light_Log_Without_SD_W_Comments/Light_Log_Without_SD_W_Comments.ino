#include <SPI.h>   
#include <SD.h>  

//pin range for outputs (pins 2-13)
const int lowestPin = 2;
const int highestPin = 13;


int freqGreen = 35;   //green LEDs blink 
int freqWhite = 40;   //hite LEDs blink
int rotSpeed = 28;    //green rotation
int state = 0;        //are green LEDs ON or OFF
int startPin = 3;     //pin green chaser pattern starts from
unsigned long lastTime1 = 0;  //last rotation step time
unsigned long lastTime2 = 0;  //last green blink toggle time


void setup() {
  //pins 2-13 as outputs (LEDs)
  for (int thisPin = lowestPin; thisPin <= highestPin; thisPin++) {
    pinMode(thisPin, OUTPUT);
  }

  // A0 and A1 inputs as trigger/control signals
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);

  //serial start for log
  Serial.begin(115200);
}


//forward dec. func. called before defined
void logEvent();
void runGreen();
void nbDelay();
void flicker();


void loop() {

  //run while A0 is HIGH 
  while (digitalRead(A0) == HIGH) {

    //random number 0-9 with different flicker pattern
    switch (random(0, 10)) {

      //3 flicker bursts with pauses in between
      // flicker(pin, duration, brightness)
      // nbDelay(ms)-waits ms without stopping the green LEDs from running

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

    // make sure green LEDs are still updating after sequence
    runGreen();
  }
}


// Logs to serial in: timestamp, type, pin, state
void logEvent(const char* type, int pin, const char* stateStr) {
  Serial.print(millis());
  Serial.print(",");
  Serial.print(type);
  Serial.print(",");
  Serial.print(pin);
  Serial.print(",");
  Serial.println(stateStr);
}


//green LED chaser- group of 3 lit LEDs that rotating in pins 3-12
//blinks at freqGreen rate. Called constantly so runs with other effects.
void runGreen() {
  unsigned long now = millis();

  //move the 3-LED window forward every rotSpeed ms
  if (now - lastTime1 >= (unsigned long)rotSpeed) {
    lastTime1 = now;
    startPin++;

    //after pin 12, jump back to pin 3
    if (startPin > 12) startPin = 3;

    //if LEDs ON, shift the lit group to the new position
    if (state == 1) {
      //OFF 3 pins that were previously lit
      for (int i = startPin - 1; i < startPin + 2; i++) {
        int x = i;
        if (x < 3) x = 12;   //wrap low end
        if (x > 12) x -= 10; //wrap high end back into 3-12

        analogWrite(x, 0);
        logEvent("GREEN", x, "OFF");
      }

      //ON the 3 pins at new location
      for (int i = startPin; i < startPin + 3; i++) {
        int x = i;
        if (x > 12) x -= 10;

        analogWrite(x, 255);
        logEvent("GREEN", x, "ON");
      }
    }
  }

  //green LEDs ON/OFF every freqGreen ms
  if (now - lastTime2 >= (unsigned long)freqGreen) {
    lastTime2 = now;

    if (state == 0) {
      //state was OFF so turn current 3 LED group ON
      for (int i = startPin; i < startPin + 3; i++) {
        int x = i;
        if (x > 12) x -= 10;

        analogWrite(x, 255);
        logEvent("GREEN", x, "ON");
      }
      state = 1;

    } else {
      //state was ON so turn current 3 LED group OFF
      for (int i = startPin; i < startPin + 3; i++) {
        int x = i;
        if (x > 12) x -= 10;

        analogWrite(x, 0);
        logEvent("GREEN", x, "OFF");
      }
      state = 0;
    }
  }
}


//non-blocking delay: waits for ms but keeps calling runGreen()
//so the green LEDs don't freeze while waiting
void nbDelay(unsigned long ms) {
  unsigned long t = millis();
  while (millis() - t < ms) {
    runGreen(); //green LEDs alive during wait
  }
}


//flickers pin "thisPin" for "duration" ms at "brightness" 0-255
//blinks ON/OFF in freqWhite ms intervals for the duration
//and keeps calling runGreen() so greens don't stop
void flicker(int thisPin, int duration, int brightness) {
  //calc how many ON/OFF cycles fit in the duration (cycle=freqWhite*2 ms)
  for (int i = 0; i < (duration / 70); i++) {

    //white LED ON
    analogWrite(thisPin, brightness);
    logEvent("WHITE", thisPin, "ON");

    //ON for freqWhite ms while green keeps running
    unsigned long t = millis();
    while (millis() - t < (unsigned long)freqWhite) {
      runGreen();
    }

    //white LED OFF
    analogWrite(thisPin, 0);
    logEvent("WHITE", thisPin, "OFF");

    //OFF for freqWhite ms while green keeps running
    t = millis();
    while (millis() - t < (unsigned long)freqWhite) {
      runGreen();
    }
  }
}