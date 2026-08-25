#include <SD.h>
const int micPin = A2; //microphone analog input pin
const int triggerPin = A0;// trigger input pin - must be HIGH to start logging
const int chipSelect = 10;// SD card chip select pin (not used in serial version)

void setup() {
  Serial.begin(115200);
  pinMode(triggerPin, INPUT); //set trigger pin as input
  pinMode(micPin, INPUT);  //set microphone pin as input
  pinMode(13, OUTPUT);
  Serial.println("Time(ms), Raw, dB");  // header
}

void loop() {
  if (digitalRead(triggerPin) == HIGH) {  //only log when trigger is active
    int sensorValue = analogRead(micPin); //
    float dB = 30 + (sensorValue / 1023.0) * 104; //sensorValue/1023.0: divides the raw reading by the max possible value, giving number between 0 and 1 (percentage)
                                                  //x104: scales up to a range of 0 to 104 (134-30)
                                                  //+30: shifts the whole thing up so the minimum is 30 instead of 0
    Serial.print(millis());
    Serial.print(", ");
    Serial.print(sensorValue);
    Serial.print(", ");
    Serial.println(dB);
    tone(13, 200);
    delay(200);
  }
  noTone(13);
}
                                 //CW