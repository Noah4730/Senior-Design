// Hearing sensitivity test bench
#include "variables.h"
#include "functions.h"

float hearingRecovery(float t) {                // Auditory Adaptation Recovery Function
  return 100 - 60.5 * exp(-t / 0.095) - 39.5 * exp(-t / 0.655);
}

float hearingSensitivity(unsigned long now) {   // Hearing sensitivity at time now
  if (hearingHigh) {
    if (now - hearingHighStart < hearingResetDelay) return hearingHoldValue;
    return 0;                                                                   // High volume has reset sensitivity to 0
  }
  if (hearingRested) return 100;
  return hearingRecovery((now - recoveryStart) / 1000.0);                       // Recovering over low volume
}

void trackHearing(unsigned long now) {          // Follow the product's switches between high and low volume
  bool high = (volumePhase == 1);
  if (high == hearingHigh) return;

  unsigned long changeTime = (volumePhase == -1) ? now : volumePhaseStart;
  if (high) {                                                       // when going from low to high, hold the current sensitivity
    hearingHoldValue = hearingSensitivity(now);
    hearingHighStart = changeTime;
  } else if (changeTime - hearingHighStart < hearingResetDelay) {   // If th high ended before the reset resume the recovery where it paused
    recoveryStart += changeTime - hearingHighStart;
  } else {                                                          // High reset sensitivity to 0
    hearingRested = false;
    recoveryStart = changeTime;
  }
  hearingHigh = high;
}
