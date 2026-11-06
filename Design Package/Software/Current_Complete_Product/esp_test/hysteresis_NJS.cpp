// Hysteresis functions
#include "variables.h"
#include "functions.h"
#include "testbench_rename.h"      // Redirect the product's pin reads/writes to the test bench

// Noah Schatz - grouping mic samples
void soundLimiter(int raw) {
  micMin = min(micMin, raw);
  micMax = max(micMax, raw);
  micSum += raw;
  micCount++;
  if (millis() - lastMicWindow < micWindow) return;

  lastMicWindow = millis();
  float level = micSum / (float)micCount;
  micReadingDb = 30 + (level / 4095.0) * 104;            // Same conversion as Christina's mic log
  checkSoundLimit(micReadingDb);

  micMin = 4095;
  micMax = 0;
  micSum = 0;
  micCount = 0;
}

// Noah Schatz - sound limit
void checkSoundLimit(float dB) {
  if (volumePhase != 1) return;                           // Low phases say nothing about high phase loudness

  if (dB >= soundLimitDb) {                               // if over limit reduce
    soundLimited = true;
    quietReadings = 0;
    volumePhaseDuration = millis() - volumePhaseStart;    // volume() ends this high phase on its next call
  } else if (dB > soundReleaseDb) {                       // if sound is more than the release Db keep the current state
    quietReadings = 0;
  } else if (++quietReadings >= requiredQuietReadings) {  // if quiet for long enough reset
    soundLimited = false;
  }
}
