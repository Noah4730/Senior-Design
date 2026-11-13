// Speaker functions
#include "variables.h"
#include "functions.h"
#include "testbench_rename.h"      // Redirect the product's pin reads/writes to the test bench

// Noah Schatz - sound control routine
void speaker() {                                     // Control the speaker volume, frequency, and sweep
  if (digitalRead(triggerPin) == LOW) return;

  int sensorValue = analogRead(soundSensorPin);  // Christina's mic read (her serial prints are left out so the plotter stays readable)
  float dB = 30 + (sensorValue / 4095.0) * 104;

  safetyLoop(dB);                               // Christina Walker - sound safety
  soundLimiter(sensorValue);                    // Noah Schatz - sound limit hysteresis on the mic reading above
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
    // tone(speakerPin, speakerFreq);
    if (safetyGain <= 0) noTone(speakerPin);
    else tone(speakerPin, speakerFreq);
  }
}

void volume() {              // Alternate random off and high volume phases, steering the running RMS toward RMSout
  unsigned long time = millis();

  if (volumePhase == -1) {
    outputSquareSum = 0;
    outputTime = 0;
  } else {
    // if (time - volumePhaseStart < volumePhaseDuration) return;           // if the current volume phase is still running
    if (time - volumePhaseStart < volumePhaseDuration) {                 // if the current volume phase is still running
      analogWrite(volumePin, ((long)volumeLevel * safetyGain) / 255);
      return;
    }

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
  // analogWrite(volumePin, volumeLevel);
  analogWrite(volumePin, ((long)volumeLevel * safetyGain) / 255);
}
