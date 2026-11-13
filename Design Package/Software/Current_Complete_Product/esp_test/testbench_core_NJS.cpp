// Trigger input (real or simulated), model updates, and Serial Plotter output

#include "variables.h"
#include "functions.h"

int benchTrigger() {                            // Real trigger pin, or the simulated trigger when simulateTrigger is true
  if (!simulateTrigger) return digitalRead(triggerPin);
  if (simTriggerEnd > 0 && millis() >= simTriggerEnd) return LOW;    // Simulated trigger release
  return (millis() < simTriggerStart) ? LOW : HIGH;                  // Simulated trigger
}

int benchDigitalRead(int pin) {                 // Every trigger check in the product also updates the test bench
  benchUpdate();
  if (pin != triggerPin) return digitalRead(pin);
  return benchTrigger();
}


void benchUpdate() {                            // Track the models and print a point every plotIntervals[graph - 1] ms
  unsigned long now = millis();
  trackHearing(now);                            // Models keep tracking after release so they pick up correctly on the next pull
  trackLight(now);

  readGraphChoice();                            // Graph choice works at any time
  if (now - lastPlot < plotIntervals[graph - 1]) return;
  lastPlot = now;
  plot(now);
}

void readGraphChoice() {                        // Send 1-5 from the Serial Plotter to switch graphs
  while (Serial.available() > 0) {
    char c = Serial.read();
    if (c >= '1' && c <= '5') graph = c - '0';
  }
}


void plot(unsigned long now) {                  // Print one point of the selected graph
  int volumeNow = (volumePhase == -1) ? 0 : volumeLevel;    // Volume PWM playing right now
  float rms = runningRms(now);

  switch (graph) {
    case 1:                                                  // RMS output value for the speaker
      printValue("Volume_PWM", volumeNow, false);
      printValue("Running_RMS", rms, false);
      printValue("RMS_Target", RMSout, true);
      break;

    case 2:                                                  // Average dB coming out of the speaker and the safety limits
      printValue("Average_dB", volumeToDb(rms), false);
      printValue("Speaker_dB", volumeToDb(volumeNow), false);
      printValue("Safety_Limit_dB", maxSpeakerDb, false);
      printValue("Exposure_Limit_dB", exposureLimitDb, true);
      break;

    case 3:                                                  // Hearing sensitivity over the high volume and speaker off intervals
      printValue("Hearing_Sensitivity", hearingSensitivity(now), false);
      printValue("High_Volume", (volumePhase == 1) ? 100 : 0, true);
      break;

    case 4:                                                  // Pupil diameter
      printValue("Pupil_mm", pupilDiameter(now), false);
      printValue("Light_Target_mm", lightOn ? pupilConstricted : pupilBaseline, true);
      break;

    case 5:                                                  // Sound limit hysteresis on the mic reading
      printValue("Mic_dB", micReadingDb, false);
      printValue("Speaker_dB", volumeToDb(volumeNow), false);
      printValue("Trip_dB", soundLimitDb, false);
      printValue("Release_dB", soundReleaseDb, false);
      printValue("Limited", soundLimited ? 140 : 40, true);  // Steps up to 140 while the limit is active
      break;
  }
}

void printValue(const char* label, float value, bool last) {
  Serial.print(label);
  Serial.print(":");
  Serial.print(value, 3);
  if (last) Serial.println();
  else Serial.print(",");
}
