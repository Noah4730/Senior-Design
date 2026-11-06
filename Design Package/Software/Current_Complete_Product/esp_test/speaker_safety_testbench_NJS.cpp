// Simulated mic, silent speaker, running RMS, and speaker dB

#include "variables.h"
#include "functions.h"

int benchAnalogRead(int pin) {                  // Speaker dB plus the room offset and noise
  if (pin != soundSensorPin) return analogRead(pin);

  unsigned long now = millis();
  int volumeNow = (volumePhase == -1) ? 0 : volumeLevel;
  float roomDb = (now >= simLoudStart && now < simLoudEnd) ? simLoudRoomDb : simQuietRoomDb;
  float noiseDb = random(-100, 101) / 100.0 * simMicNoiseDb;
  float dB = volumeToDb(volumeNow) + roomDb + noiseDb;
  return constrain((int)((dB - 30) / 104.0 * 4095), 0, 4095);       // Inverse of the product's raw to dB conversion
}

void benchTone(int, int) {}                     // The speaker stays silent
void benchNoTone(int) {}


float runningRms(unsigned long now) {           // The product's running RMS
  if (volumePhase == -1) return 0;

  unsigned long played = min(now - volumePhaseStart, volumePhaseDuration);
  float squareSum = outputSquareSum + (float)volumeLevel * volumeLevel * played;
  unsigned long totalTime = outputTime + played;
  if (totalTime == 0) return volumeLevel;
  return sqrt(squareSum / totalTime);
}

float volumeToDb(float level) {                 // Speaker dB for a volume PWM level
  if (level <= 0) return speakerOffDb;
  if (level <= LVmax) return LVminDb + (level - LVmin) * (LVmaxDb - LVminDb) / (LVmax - LVmin);
  return LVmaxDb + (level - LVmax) * (maxSpeakerDb - LVmaxDb) / (255.0 - LVmax);
}
