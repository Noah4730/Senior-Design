// sound log- microphone level log to SD card
// reads analog microphone input and converts raw reading
//  into decibel value, then logs it to a file on SD
//  card ("log.txt") every 200ms, but only while trigger pin is HIGH.
//  each logged line is a timestamp (ms since
//  boot), the raw analog reading (0–1023), and the calculated dB estimate.
//  unlike the light strobe log ( opens file once in setup() and
//  keeps it open whole time), this opens and closes log.txt
//  every single time it logs a reading. Its slower, but the
//  file is always saved/closed between writes. If power is cut
//  mid use, entire log is not lost, just possibly last reading.

#include <SD.h>  //library for reading/writing files on SD card

const int micPin= A2;   //microphone analog input pin
const int triggerPin= A0;  //trigger input pin must be HIGH to start logging
const int chipSelect = 10;   // SD card chip select pin (not used in serial version)

File myFile;   // reused each time log.txt is opened/closed
// set up runs once at power on/ reset

void setup() {
  Serial.begin(9600);     //open serial status/debug messages can be printed

  pinMode(triggerPin, INPUT);    //set trigger pin as input
  pinMode(micPin, INPUT);       //set microphone pin as input

  Serial.println("Initializing SD card.");

  // initialize SD card. If it fails, then
  // print error and stop program (nothing can be logged)
  if (!SD.begin(chipSelect)) {
    Serial.println("SD card failed");
    while (1);   //stop everything, do nothing else
  }
  Serial.println("SD card ready.");

  //delete old file
  // remove previous log.txt so run starts with clean file
  // instead of adding onto data from previous session

  SD.remove("log.txt");

  //file with header
  //open new log.txt to write CSV header row
  // (column names), then close it again

  myFile= SD.open("log.txt", FILE_WRITE);
  if (myFile) {
    myFile.println("Time(ms), Raw, dB");
    myFile.close();
  }
}

// main
// every pass through loop(), check whether trigger active. If active,
// take microphone reading, convert it to approximate dB value,
// add one line to log.txt, then wait 200ms before next reading
// if trigger is LOW, loop()  keeps checking.

void loop() {
  if (digitalRead(triggerPin)== HIGH) { //only log when trigger is active

    //read raw microphone value (0-1023) representing
    // the analog voltage on micPin.
    int sensorValue= analogRead(micPin);

    //convert the raw 0-1023 reading into an approximate dB value in
    // range of 30-134 dB. This is a simple linear mapping, not a true
    // calculated dB measurement:
    float dB= 30+ (sensorValue/ 1023.0)* 104;  //sensorValue/1023.0: divides raw reading by max possible value, giving number between 0 and 1 (percentage)
                                                  //x104: scales up to range of 0 to 104 (134-30)
                                                  //+30: shifts the whole thing so the minimum is 30 instead of 0

    //open log.txt fresh, write (timestamp, raw value, dB value),
    // close so the write is saved to the card
    myFile= SD.open("log.txt", FILE_WRITE);
    if (myFile) {
      myFile.print(millis());  //elapsed time in ms since boot
      myFile.print(", ");
      myFile.print(sensorValue);  //raw analog reading (0-1023)
      myFile.print(", ");
      myFile.println(dB);    //calculated dB estimate
      myFile.close();
    }

    delay(200);   //wait 200ms before taking next reading, so log.txt
                  // gets about 5 readings per second instead of flooding
                  // SD card with writes
  }
}

                                         //CW

