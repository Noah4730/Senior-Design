// Logging functions
#include "variables.h"
#include "functions.h"

// Christina Walker - safety and logging helper
void logEvent(const char* type, int pin, const char* stateStr) {      // Log events to serial monitor

  if(digitalRead(triggerPin) == LOW) return;

  Serial.print(millis());
  Serial.print(",");
  Serial.print(type);
  Serial.print(",");
  Serial.print(pin);
  Serial.print(",");
  Serial.println(stateStr);
}
