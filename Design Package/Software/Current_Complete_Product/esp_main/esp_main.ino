#include "variables.h"        // All constants and global variables
#include "functions.h"        // Function prototypes (functions live in the .cpp files)




// Christina Walker - safety/input setup and data-logging behavior
void setup() {                                                        // Initialize pins
  for (int i = 0; i < numGreenPins; i++) {
    pinMode(greenPins[i], OUTPUT);
  }
  for (int i = 0; i < numWhitePins; i++) {
    pinMode(whitePins[i], OUTPUT);
  }
  pinMode(speakerPin, OUTPUT);
  pinMode(volumePin, OUTPUT);
  pinMode(triggerPin, INPUT);
  pinMode(soundSensorPin, INPUT);

  analogWrite(volumePin, volumeLevel);
  Serial.begin(115200);
}


// Noah Schatz - main light and sound trigger loop
void loop() {                           // Main loop with white light algorithm and major function calls

  while (digitalRead(triggerPin) == HIGH) {     // Trigger pin activates light routine

    flicker(random(minimumFlicker, maximumFlicker + 1));     // Flicker white for a random duration
    sysTime(random(minimumPause, maximumPause + 1));         // Pause white for a random duration

    runGreen();
    speaker();
  }

  setWhite(0);                                                          // Turn off all lights when trigger pin is LOW
  for (int i = 0; i < numGreenPins; i++) {
    analogWrite(greenPins[i], 0);
  }
  noTone(speakerPin);
  analogWrite(volumePin, 0);
  volumePhase = -1;
  soundLimited = false;                         // Each trigger pull starts at full output
  state = 0;
  whiteStep = 0;

  // Christina Walker - safety/data logging readout
  int sensorValue = analogRead(soundSensorPin); //
  float dB = 30 + (sensorValue / 4095.0) * 104; //sensorValue/4095.0 (ESP32 12-bit ADC): divides the raw reading by the max possible value, giving number between 0 and 1 (percentage)
                                                //x104: scales up to a range of 0 to 104 (134-30)
                                                //+30: shifts the whole thing up so the minimum is 30 instead of 0

  // Christina Walker - serial logging output for safety monitoring
  Serial.print(millis());
  Serial.print(", ");
  Serial.print(sensorValue);
  Serial.print(", ");
  Serial.println(dB);

}
