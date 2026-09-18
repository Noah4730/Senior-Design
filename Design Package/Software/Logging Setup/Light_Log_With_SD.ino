#include <SPI.h>
#include <SD.h>

const int chipSelect = 10;
File logFile;

const int lowestPin = 2;
const int highestPin = 13;

int freqGreen = 35;
int freqWhite = 40;
int rotSpeed = 28;
int state = 0;
int startPin = 3;
unsigned long lastTime1 = 0;
unsigned long lastTime2 = 0;
unsigned long lastFlush = 0;



void setup() {

  for (int thisPin = lowestPin; thisPin <= highestPin; thisPin++) {
    pinMode(thisPin, OUTPUT);
  }

  pinMode(A0, INPUT);
  pinMode(A1, INPUT);

  Serial.begin(9600);

  if (!SD.begin(chipSelect)) {
    Serial.println("SD init failed");
    while (1);
  }

  logFile = SD.open("strobe.txt", FILE_WRITE);
  if (!logFile) {
    Serial.println("File open failed");
    while (1);
  }

  logFile.println("millis,type,pin,state");
  logFile.flush();
}

void runGreen();
void nbDelay();
void flicker();


void loop() {

  while (digitalRead(A0) == HIGH) {

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

    runGreen();

    if (millis() - lastFlush >= 500) {
      logFile.flush();
      lastFlush = millis();
    }
  }
}


void runGreen() {
  unsigned long now = millis();

  if (now - lastTime1 >= (unsigned long)rotSpeed) {
    lastTime1 = now;
    startPin++;
    if (startPin > 12) startPin = 3;
    if (state == 1) {
      for (int i = startPin - 1; i < startPin + 2; i++) {
        int x = i;
        if (x < 3) x = 12;
        if (x > 12) x -= 10;
        analogWrite(x, 0);
        logFile.print(millis()); logFile.print(",GREEN,"); logFile.print(x); logFile.println(",OFF");
      }
      for (int i = startPin; i < startPin + 3; i++) {
        int x = i;
        if (x > 12) x -= 10;
        analogWrite(x, 255);
        logFile.print(millis()); logFile.print(",GREEN,"); logFile.print(x); logFile.println(",ON");
      }
    }
  }

  if (now - lastTime2 >= (unsigned long)freqGreen) {
    lastTime2 = now;
    if (state == 0) {
      for (int i = startPin; i < startPin + 3; i++) {
        int x = i;
        if (x > 12) x -= 10;
        analogWrite(x, 255);
        logFile.print(millis()); logFile.print(",GREEN,"); logFile.print(x); logFile.println(",ON");
      }
      state = 1;
    } else {
      for (int i = startPin; i < startPin + 3; i++) {
        int x = i;
        if (x > 12) x -= 10;
        analogWrite(x, 0);
        logFile.print(millis()); logFile.print(",GREEN,"); logFile.print(x); logFile.println(",OFF");
      }
      state = 0;
    }
  }
}

void nbDelay(unsigned long ms) {
  unsigned long t = millis();
  while (millis() - t < ms) {
    runGreen();
  }
}

void flicker(int thisPin, int duration, int brightness) {
  for (int i = 0; i < (duration / 70); i++) {
    analogWrite(thisPin, brightness);
    logFile.print(millis()); logFile.print(",WHITE,"); logFile.print(thisPin); logFile.println(",ON");

    unsigned long t = millis();
    while (millis() - t < (unsigned long)freqWhite) {
      runGreen();
    }

    analogWrite(thisPin, 0);
    logFile.print(millis()); logFile.print(",WHITE,"); logFile.print(thisPin); logFile.println(",OFF");

    t = millis();
    while (millis() - t < (unsigned long)freqWhite) {
      runGreen();
    }
  }
}
