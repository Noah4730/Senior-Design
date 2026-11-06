// white LED tracking and the pupil response model

#include "variables.h"
#include "functions.h"

void benchAnalogWrite(int pin, int value) {     // Record white LED writes for the pupil model
  bool lit = false;
  for (int i = 0; i < numWhitePins; i++) {
    if (pin == whitePins[i]) whiteLevels[i] = value;
    if (whiteLevels[i] > 0) lit = true;
  }
  if (lit && !whiteLit) whiteLitSince = millis();
  if (!lit && whiteLit) whiteDarkSince = millis();
  whiteLit = lit;
}


void trackLight(unsigned long now) {                                // Pupil response
  if (whiteLit && !lightOn) {                                       // First white flash after a pause
    lightOn = true;
    lightOnSince = whiteLitSince;
  } else if (!whiteLit && lightOn && now - whiteDarkSince > (unsigned long)freqWhite + 5) {   // Dark longer than freqWhit + 5 then the white light has paused
    lightOn = false;
    if (pupilConstricting) startPupilResponse(false, max(whiteDarkSince + freqWhite, pupilResponseStart));   // The pause began when the last flash's off time ended
  }

  if (lightOn && !pupilConstricting && now - lightOnSince >= pupilDelay) {
    startPupilResponse(true, lightOnSince + pupilDelay);
  }
}

float pupilConstriction(float t) {              // Constriction progress 0 to 1
  return 1 - (1 + 7.7 * t) * exp(-7.7 * t);
}

float pupilRecovery(float t) {                  // Recovery progress 0 to 1
  return 1 - 0.65 * exp(-0.83 * t) - 0.35 * exp(-0.14 * t);
}

float pupilDiameter(unsigned long now) {        // each constriction and dilating phase picks up from the current diameter
  float t = (now - pupilResponseStart) / 1000.0;
  if (pupilConstricting) return pupilStartDiameter + (pupilConstricted - pupilStartDiameter) * pupilConstriction(t);
  return pupilStartDiameter + (pupilBaseline - pupilStartDiameter) * pupilRecovery(t);
}

void startPupilResponse(bool constricting, unsigned long time) {
  pupilStartDiameter = pupilDiameter(time);
  pupilResponseStart = time;
  pupilConstricting = constricting;
}
