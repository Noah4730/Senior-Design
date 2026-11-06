// White light functions
#include "variables.h"
#include "functions.h"
#include "testbench_rename.h"      // Redirect the product's pin reads/writes to the test bench

// Noah Schatz - non-blocking timing helper for light/sound sequencing
void sysTime(unsigned long ms) {          // Delay function based on system time that allows semi parallel functions
  if(digitalRead(triggerPin) == LOW) return;

  unsigned long t = millis();
  while (millis() - t < ms) {
    if(digitalRead(triggerPin) == LOW) return;
    runGreen();
    speaker();
  }
}

// Noah Schatz - set every white LED pin to the same brightness
void setWhite(int brightness) {
  for (int i = 0; i < numWhitePins; i++) {
    analogWrite(whitePins[i], brightness);
  }
}

// Noah Schatz - turn on the white pins for the current flash of the selected flicker mode
void showWhiteStep() {
  if (flickerMode < 1 || flickerMode > 3) {               // Mode 0 (or invalid): all pins at once
    setWhite(whiteBrightness);
    return;
  }

  for (int j = 0; j < 3; j++) {
    int pinNumber = whitePatterns[flickerMode - 1][whiteStep][j];
    if (pinNumber > 0) analogWrite(whitePins[pinNumber - 1], whiteBrightness);
  }

  whiteStep++;                                             // Advance to the next flash in the pattern
  if (whiteStep >= whitePatternSteps) whiteStep = 0;
}

// Noah Schatz - white LED strobe/flicker routine
void flicker(int duration) {  // Flicker white pins for a certain duration at whiteBrightness using flickerMode

  if(digitalRead(triggerPin) == LOW) return;

  for (int i = 0; i < (duration / (2 * freqWhite)); i++) {     // Whole on/off flashes that fit in duration

    if(digitalRead(triggerPin) == LOW) return;

    showWhiteStep();
    // logEvent("WHITE", whitePins[0], "ON");

    unsigned long t = millis();
    while (millis() - t < (unsigned long)freqWhite) {
      if(digitalRead(triggerPin) == LOW) { setWhite(0); return; }
      runGreen();
      speaker();
    }

    setWhite(0);
    // logEvent("WHITE", whitePins[0], "OFF");

    t = millis();
    while (millis() - t < (unsigned long)freqWhite) {
      if(digitalRead(triggerPin) == LOW) return;
      runGreen();
      speaker();
    }
  }
}
