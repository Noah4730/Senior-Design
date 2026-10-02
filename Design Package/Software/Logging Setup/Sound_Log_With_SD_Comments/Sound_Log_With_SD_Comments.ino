#include <SD.h>

/*
  Christina Walker
  Data logging and safety-related functionality
*/

const int micPin = A2; //microphone analog input pin
const int triggerPin = A0; // trigger input pin - must be HIGH to start logging
const int chipSelect = 10; // SD card chip select pin (not used in serial version)

File myFile;
void setup() {
  Serial.begin(9600);
  pinMode(triggerPin, INPUT); //set trigger pin as input
  pinMode(micPin, INPUT); //set microphone pin as input
  Serial.println("Initializing SD card.");
  if (!SD.begin(chipSelect)) {
    Serial.println("SD card failed");
    while (1);
  }
  Serial.println("SD card ready.");
  //delete old file
  SD.remove("log.txt");
  //file with header
  myFile = SD.open("log.txt", FILE_WRITE);
  if (myFile) {
    myFile.println("Time(ms), Raw, dB");
    myFile.close();
  }
}
void loop() {
  if (digitalRead(triggerPin) == HIGH) { //only log when trigger is active
    int sensorValue = analogRead(micPin);
    float dB = 30 + (sensorValue / 1023.0) * 104; //sensorValue/1023.0: divides the raw reading by the max possible value, giving number between 0 and 1 (percentage)
                                                  //x104: scales up to a range of 0 to 104 (134-30)
                                                  //+30: shifts the whole thing up so the minimum is 30 instead of 0
    myFile = SD.open("log.txt", FILE_WRITE);
    if (myFile) {
      myFile.print(millis());
      myFile.print(", ");
      myFile.print(sensorValue);
      myFile.print(", ");
      myFile.println(dB);
      myFile.close();
    }
    delay(200);
  }
}
   
                                         //CW
