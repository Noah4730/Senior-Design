// Speaker functions
#include "variables.h"
#include "functions.h"

// Noah Schatz - sound control routine
void speaker() {                                     // Control the speaker volume, frequency, and sweep
  if (digitalRead(triggerPin) == LOW) return;

  // Christina Walker - data logging during sound activation
  int sensorValue = analogRead(soundSensorPin); //
  float dB = 30 + (sensorValue / 4095.0) * 104; //sensorValue/4095.0 (ESP32 12-bit ADC): divides the raw reading by the max possible value, giving number between 0 and 1 (percentage)
                                                //x104: scales up to a range of 0 to 104 (134-30)
                                                //+30: shifts the whole thing up so the minimum is 30 instead of 0

  // Christina Walker - serial output for sound-trigger logging
  Serial.print(millis());
  Serial.print(", ");
  Serial.print(sensorValue);
  Serial.print(", ");
  Serial.println(dB);

  soundLimiter(sensorValue);                    // Noah Schatz - sound limit on the mic reading
  volume();
  if (volumePhase != 1) return;                                        // Speaker stays off between high phases

  unsigned long time = millis();

  if (time - lastTime3 >= sweep) {                                     // Pick a new random frequency every sweep milliseconds
    lastTime3 = time;
    int mappedFreq = map(volumeLevel, LVmin, HVmax, minimumFrequency, maximumFrequency);   // Map volume PWM to frequency
    mappedFreq = constrain(mappedFreq, minimumFrequency, maximumFrequency);
    int lowFreq = max(mappedFreq - frequencyOffset, minimumFrequency);    // Keep the random offset inside the frequency range
    int highFreq = min(mappedFreq + frequencyOffset, maximumFrequency);
    speakerFreq = random(lowFreq, highFreq + 1);
    tone(speakerPin, speakerFreq);
  }
}

void volume() {              // Alternate random off and high volume phases, steering the running RMS toward RMSout
  unsigned long time = millis();

  if (volumePhase == -1) {
    outputSquareSum = 0;
    outputTime = 0;
  } else {
    if (time - volumePhaseStart < volumePhaseDuration) return;           // if the current volume phase is still running

    outputSquareSum += (float)volumeLevel * volumeLevel * volumePhaseDuration;   // Add the finished phase to the running RMS
    outputTime += volumePhaseDuration;
  }

  if (volumePhase == 1) {                                                 // After a high phase, turn the speaker off for a random interval
    volumePhase = 0;
    volumeLevel = 0;
    volumePhaseDuration = random(minimumOffDuration, maximumOffDuration + 1);
    noTone(speakerPin);

  } else {                                                                // After an off phase play a random high volume phase
    volumePhase = 1;
    float currentRMS = 0;
    if (outputTime > 0) currentRMS = sqrt(outputSquareSum / outputTime);

    int middleHV = (HVmin + HVmax) / 2;
    if (currentRMS > RMSout || soundLimited) {                            // Above RMSout or sound limited: use the quieter half of the high range
      volumeLevel = random(HVmin, middleHV + 1);
    } else {                                                              // At or below RMSout: use the louder half of the high range
      volumeLevel = random(middleHV, HVmax + 1);
    }
    volumePhaseDuration = random(minimumHVDuration, maximumHVDuration + 1);
  }

  volumePhaseStart = time;
  analogWrite(volumePin, volumeLevel);
}
