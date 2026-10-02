#include <SPI.h>   // needed for SPI bus
#include <SD.h>    // library for reading/writing files on the SD card

//sd card/logging set up

const int chipSelect= 10;   //pin connected to the SD card modules CS (chip select) line
File logFile;   //file object kept open for whole run and write log lines to

// LED pin range 
// pins 2 through 13 are  set up as digital outputs. Pin 2 is used by the
// white strobe. pins 3–12 are used by the green strobes rotating 3 LED band.

const int lowestPin= 2;
const int highestPin= 13;

//green strobe timing/state variables

int freqGreen= 35;  //how often (ms) the green band toggles ON/OFF (blink rate)
int rotSpeed= 28;   // how often (ms) the green band shifts over by one pin (rotation speed)
int state= 0;    //current blink state of the green band: 0= OFF, 1= ON
int startPin= 3;   //the pin where the current 3 LED green band currently starts

//  last time we did x socan measure elapsed time without
// blocking (standard Arduino "millis() timer" pattern).

unsigned long lastTime1= 0;  // last time green bands rotationposition updated
unsigned long lastTime2= 0;  // last time green bands blink state toggled
unsigned long lastFlush= 0;  // last time flushed/saved the log file to the SD card

//white strobe timing

int freqWhite= 40;  // half period (ms) of each flicker() on/off pulse aka flicker() below

//setup- runs once at power on/reset

void setup() {

  //configure every LED pin (2 through 13) as an output so we can drive them
  // with digitalWrite()/analogWrite().

  for (int thisPin= lowestPin; thisPin <= highestPin; thisPin++) {
    pinMode(thisPin,OUTPUT);
  }

  // A0 is the trigger input- when this reads HIGH, the strobe show runs.
  // A1 is reserved here (e.g. for a mic input in a related code) but isn't being
  // actively used in this particular code file

  pinMode(A0, INPUT);
  pinMode(A1, INPUT);

  Serial.begin(9600);  //open serial so we can print error messages for debugging

  //try to initialize the SD card. If it fails (card missing, wiring issue,
  // wrong chipSelect pin, etc.), print an error and stop the program.
  // can't log anything.

  if (!SD.begin(chipSelect)) {
    Serial.println("SD init failed");
    while (1);   // Infinite loop= stop everything, do nothing else
  }

  //open strobe.txt on SD card in write mode. FILE_WRITE
  // appends to the end of the file if it already exists

  logFile= SD.open("strobe.txt",FILE_WRITE);
  if (!logFile) {
    Serial.println("File open failed");
    while (1);   //stop here if log file can't be opened
  }

  //header row so the log file is easier to read/import later 
  
  logFile.println("millis,type,pin,state");
  logFile.flush();   //force header out to physical SD card immediately
}

//  runGreen()-updates the green rotating strobe by one tick
//  function called very frequently (basically every spare moment,
//  including from inside the white strobes waiting periods) so the green
//  strobe animation stays smooth no matter what else is happening. It does
//  not block/delay.  It checks if enough time has passed and if so,
//  updates the LEDs and immediately returns.
//  there are two independent behaviors happening, each on its own timer:
//    (1) rotation- every rotSpeed ms, the 3 LED lit band slides over by
//        one position (wrapping around from pin 12 back to pin 3).
//    (2) blinking- every freqGreen ms, the entire current band toggles
//        between fully ON and fully OFF.

void runGreen() {
  unsigned long now= millis();   //grab current elapsed time once per call

  // (1) rotation: has enough time passed to shift band over?

  if (now - lastTime1>= (unsigned long)rotSpeed) {
    lastTime1= now;    // reset rotation timer
    startPin++;     // move bands starting pin forward by one
    if (startPin> 12) startPin= 3;   //wrap back around to pin 3 after pin 12

    // only actually move the visible band if its currently supposed to be
    // lit (state== 1). If its currently OFF, there's nothing to slide
    // on screen, so rotation just updates startPin for next time.

    if (state== 1) {

      // turn OFF the bands old position (the 3 pins just behind the new
      // startPin) before turning ON the new position, so it looks like the
      // lit band is sliding rather than just growing.

      for (int i= startPin - 1; i< startPin + 2; i++) {
        int x= i;
        if (x< 3) x= 12;   // wrap below pin 3 around to pin 12
        if (x> 12) x -= 10;  // wrap above pin 12 back down into range
        analogWrite(x, 0);  //turn this LED off
        //log OFF event: timestamp, effect="GREEN", which pin, state
        logFile.print(millis()); logFile.print(",GREEN,"); logFile.print(x); logFile.println(",OFF");
      }

      // turn ON the bands new position (3 pins starting at the new startPin)
      for (int i= startPin; i< startPin + 3; i++) {
        int x= i;
        if (x> 12) x -= 10;  // wrap above pin 12 back down into range
        analogWrite(x, 255);  //turn this LED on fully
        //log ON event
        logFile.print(millis()); logFile.print(",GREEN,"); logFile.print(x); logFile.println(",ON");
      }
    }
  }

  //(2) blinking: has enough time passed to toggle entire band?

  if (now - lastTime2>= (unsigned long)freqGreen) {
    lastTime2= now;   //reset blink timer

    if (state== 0) {
      //currently OFF aka turn the whole current 3 LED band ON
      for (int i= startPin; i< startPin + 3; i++) {
        int x= i;
        if (x> 12) x -= 10;
        analogWrite(x, 255);
        logFile.print(millis()); logFile.print(",GREEN,"); logFile.print(x); logFile.println(",ON");
      }
      state= 1;  //in the ON state now
    } else {
      //currently ON aka turn the whole current 3 LED band OFF
      for (int i= startPin; i< startPin + 3; i++) {
        int x= i;
        if (x> 12) x -= 10;
        analogWrite(x, 0);
        logFile.print(millis()); logFile.print(",GREEN,"); logFile.print(x); logFile.println(",OFF");
      }
      state= 0;  //now in OFF state
    }
  }
}

//  nbDelay(ms)-non-blocking delay
//  replacement for normal delay(ms). Instead of just
//  freezing the whole program for ms milliseconds, instead busy-waits in a
//  loop and calls runGreen() on every pass so green rotating strobe
//  keeps moving smoothly even while waiting
//  white strobe pattern below needs pauses between bursts, and
//   do not want green strobe to visibly freeze during these
//  pauses.

void nbDelay(unsigned long ms) {
  unsigned long t= millis();   //remember when wait started
  while (millis() - t< ms) {  //keep looping until ms has elapsed
    runGreen();      //let green strobe keep updating
  }
}

//  flicker(pin, duration, brightness- drives white strobes flicker burst
//  rapid on/off flicker on pin for duration
//  milliseconds total  at a rate set by freqWhite (bigger freqWhite= slower
//  flicker). Like nbDelay, pauses between each on/off toggle
//  are done with manual millis() based wait loop that also calls runGreen(),
//  so green strobe does not stall while white LED is mid flicker.
//  duration/ 70-how many on/off flicker cycles to run. This
//  is an approximation, not an exact timer. Real elapsed time depends
//  on freqWhite, but duration/70 keeps the number of flickers
//  proportional to how long burst is made to last.

void flicker(int thisPin, int duration, int brightness) {
  for (int i= 0; i< (duration/ 70); i++) {

    //ON phase
    analogWrite(thisPin, brightness);   //turn white LED on to given brightness
    logFile.print(millis()); logFile.print(",WHITE,"); logFile.print(thisPin); logFile.println(",ON");

    unsigned long t= millis();
    while (millis() - t< (unsigned long)freqWhite) {
      runGreen();  //keep green strobe moving while holding ON phase
    }

    //OFF phase
    analogWrite(thisPin, 0);    //turn white LED back off
    logFile.print(millis()); logFile.print(",WHITE,"); logFile.print(thisPin); logFile.println(",OFF");

    t= millis();
    while (millis() - t< (unsigned long)freqWhite) {
      runGreen();   //keep green strobe moving while holding OFF phase
    }
  }
}


//  loop()
// while trigger pin (A0) is held HIGH, repeatedly:
//    1. picks random white strobe pattern (10 different
//       sequences of flicker() bursts and nbDelay() pauses, case 0–9)
//       randomizes which pattern plays next and keeps the
//       flicker effect feeling unpredictable rather than looping
//       in a continuous repeating cycle 
//    2. calls runGreen() directly (on top of all the calls that
//       already happened inside flicker()/nbDelay()) to make sure
//       green strobe gets called right after each pattern ends.
//    3. flushes log file regularly to SD card (every 500ms) so
//       log data gets saved, without flushing on
//       every single line (slower).
//  each case is different sequence of:
//      flicker(pin 2, burst length-ms, full brightness) and nbDelay(pause-ms)
//  repeated 3 times, with different numbers picked

void loop() {

  while (digitalRead(A0)== HIGH) {   // run while trigger is active

    //randomly choose one 10 white strobe patterns (0-9)
    switch (random(0, 10)) {

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

    runGreen();  //give green strobe one more update pass after each pattern

    // write/flush log to SD card twice every second
    // flushing forces buffered writes out to the actual card. Doing too
    // often is slow
    if (millis() - lastFlush >= 500) {
      logFile.flush();
      lastFlush= millis();
    }
  }
}
