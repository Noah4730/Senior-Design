// Green light functions
#include "variables.h"
#include "functions.h"

// Noah Schatz - green LED light routine
void runGreen() {                                     // Run green light routine
  unsigned long now = millis();

  if(digitalRead(triggerPin) == LOW) return;                  // If trigger pin is LOW, exit function

  if (now - lastTime1 >= (unsigned long)rotSpeed) {             // Rotate the green lights every rotSpeed milliseconds
    lastTime1 = now;
    analogWrite(greenPins[greenIndex], 0);                      // Turn off the current pin before moving to the next one
    // logEvent("GREEN", greenPins[greenIndex], "OFF");

    greenIndex++;
    if (greenIndex >= numGreenPins) greenIndex = 0;

    if (state == 1) {                                           // If green lights are on, turn on only the next pin
      analogWrite(greenPins[greenIndex], greenBrightness);
      // logEvent("GREEN", greenPins[greenIndex], "ON");
    }
  }

  if (now - lastTime2 >= (unsigned long)freqGreen) {       // Toggle the active green light on and off every freqGreen milliseconds
    lastTime2 = now;

    if (state == 0) {
      analogWrite(greenPins[greenIndex], greenBrightness);
      // logEvent("GREEN", greenPins[greenIndex], "ON");
      state = 1;

    } else {
      analogWrite(greenPins[greenIndex], 0);
      // logEvent("GREEN", greenPins[greenIndex], "OFF");
      state = 0;
    }
  }
}
