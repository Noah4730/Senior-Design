/*
  Test bench for Current_Complete_Product.ino (test case SLV-03)
*/

#include "variables.h"        // Test bench setup values, product variables, and test bench state
#include "functions.h"        // Function prototypes

#define setup productSetup
#define loop productLoop
#include "testbench_rename.h"      // Redirect the product's pin reads/writes to the test bench

// ==================================== Current_Complete_Product.ino ==========================================

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
}
// ======================================================= END ===================================================

#undef setup
#undef loop
#undef digitalRead
#undef analogRead
#undef analogWrite
#undef tone
#undef noTone

// ----------------------------------------------------Test Bench------------------------------------------------------------

void setup() {                                  // Start the plotter's serial port, then the product's own setup
  Serial.begin(115200);
  productSetup();
}

void loop() {                                   // Run the product
  productLoop();
  benchUpdate();
}
