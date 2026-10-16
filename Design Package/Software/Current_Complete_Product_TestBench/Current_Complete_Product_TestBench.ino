/*
  Test bench for Current_Complete_Product.ino (test case SLV-03)
*/

// ---------------- Test bench setup ----------------

const unsigned long simTriggerStart = 2000;

int graph = 1;                                  // 1 = speaker RMS, 2 = average dB, 3 = hearing sensitivity, 4 = pupil
const unsigned long plotIntervals[] = {200, 200, 50, 100};   // ms between points for graphs 1-4

const float LVminDb = 40.0;                     // Low volume min/max db
const float LVmaxDb = 80.0;
const float maxSpeakerDb = 130.0;
const float exposureLimitDb = 100.0;            // Exposure time is tracked above this level
const float speakerOffDb = 30.0;                // dB plotted while the speaker is off

const unsigned long hearingResetDelay = 100;    // ms of high volume before hearing sensitivity resets to 0

const float ledLux = 100000.0;                   // Lux at the eye while the white LEDs are on
const unsigned long pupilDelay = 200;           // ms of white light before the pupil starts constricting
const float pupilBaseline = 3.21;


int benchDigitalRead(int pin);
void benchAnalogWrite(int pin, int value);
void benchTone(int pin, int frequency);
void benchNoTone(int pin);

#define setup productSetup
#define loop productLoop
#define digitalRead benchDigitalRead
#define analogWrite benchAnalogWrite
#define tone benchTone
#define noTone benchNoTone

// ==================================== Current_Complete_Product.ino ==========================================
#include <SPI.h>
#include <SD.h>

/*
  Christina Walker: safety checks and data logging functions
  Noah Schatz: light and sound control functions
*/

// const int chipSelect = 10;
// File logFile;

const int greenPins[] = {16, 17, 18, 19, 21};
const int whitePins[] = {22, 23, 25, 26, 27};
const int numGreenPins = 5;
const int numWhitePins = 5;
const int speakerPin = 32;
const int volumePin = 33;
const int triggerPin = 34;
const int soundSensorPin = 35;
const int LVmin = 20;
const int LVmax = 80;
const int HVmin = 150;
const int HVmax = 255;
const int RMSout = 160;
const int minimumLVDuration = 250;
const int maximumLVDuration = 750;
const int minimumHVDuration = 500;
const int maximumHVDuration = 1500;
const int minimumFrequency = 3000;
const int maximumFrequency = 10000;
const int frequencyOffset = 3000;
const int minimumFlicker = 900;
const int maximumFlicker = 1200;
const int minimumPause = 100;
const int maximumPause = 200;


const int whitePatternSteps = 5;
const int whitePatterns[3][whitePatternSteps][3] = {         // White flicker patterns for modes 1-3
  {{1, 0, 0}, {3, 0, 0}, {5, 0, 0}, {2, 0, 0}, {4, 0, 0}},   // star, one pin at a time
  {{1, 2, 0}, {4, 3, 0}, {5, 1, 0}, {2, 3, 0}, {5, 4, 0}},   // two pins at a time
  {{1, 2, 3}, {3, 4, 5}, {5, 1, 2}, {2, 3, 4}, {4, 5, 1}}    // three pins at a time
};

int freqWhite = 40;
int freqGreen = 35;
int greenBrightness = 255;
int whiteBrightness = 255;
int flickerMode = 1;                            // 0 = all at once, 1 = star, 2 = two at a time, 3 = three at a time
int whiteStep = 0;                              // Current flash in the white flicker pattern
int rotSpeed = 28;
int speakerFreq = 1000;
int sweep = 50;
int volumeLevel = LVmin;
int volumePhase = -1;                           // -1 = off, 0 = low volume, 1 = high volume
int state = 0;
int greenIndex = 0;
unsigned long lastTime1 = 0;
unsigned long lastTime2 = 0;
unsigned long lastTime3 = 0;
unsigned long volumePhaseStart = 0;
unsigned long volumePhaseDuration = 0;
unsigned long outputTime = 0;                   // Total ms counted in the running RMS
float outputSquareSum = 0;                      // Sum of volumeLevel^2 * ms counted in the running RMS




// Christina Walker - safety/input setup and data-logging behavior
void setup() {                                                        // Initialize pins
  for (int i = 0; i < numGreenPins; i++) {
    pinMode(greenPins[i], OUTPUT);
  }
  for (int i = 0; i < numWhitePins; i++) {
    pinMode(whitePins[i], OUTPUT);
  }
  pinMode(speakerPin, OUTPUT);
  pinMode(volumePin, OUTPUT);
  pinMode(triggerPin, INPUT);
  pinMode(soundSensorPin, INPUT);

  analogWrite(volumePin, volumeLevel);
}


void logEvent();
void runGreen();
void sysTime();
void flicker();


// Noah Schatz - main light and sound trigger loop
void loop() {                           // Main loop with white light algorithm and major function calls

  while (digitalRead(triggerPin) == HIGH) {     // Trigger pin activates light routine

    flicker(random(minimumFlicker, maximumFlicker + 1));     // Flicker white for a random duration
    sysTime(random(minimumPause, maximumPause + 1));         // Pause white for a random duration

    runGreen();
    speaker();
  }

  setWhite(0);                                                          // Turn off all lights when trigger pin is LOW
  for (int i = 0; i < numGreenPins; i++) {
    analogWrite(greenPins[i], 0);
  }
  noTone(speakerPin);
  analogWrite(volumePin, 0);
  volumePhase = -1;
  state = 0;
  whiteStep = 0;
}

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

// Noah Schatz - sound control routine
void speaker() {                                     // Control the speaker volume, frequency, and sweep
  if (digitalRead(triggerPin) == LOW) return;

  volume();

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

void volume() {              // Alternate random low and high volume phases, steering the running RMS toward RMSout
  unsigned long time = millis();

  if (volumePhase == -1) {
    outputSquareSum = 0;
    outputTime = 0;
  } else {
    if (time - volumePhaseStart < volumePhaseDuration) return;           // if the current volume phase is still running

    outputSquareSum += (float)volumeLevel * volumeLevel * volumePhaseDuration;   // Add the finished phase to the running RMS
    outputTime += volumePhaseDuration;
  }

  if (volumePhase == 1) {                                                 // After a high phase, play a random low volume phase
    volumePhase = 0;
    volumeLevel = random(LVmin, LVmax + 1);
    volumePhaseDuration = random(minimumLVDuration, maximumLVDuration + 1);

  } else {                                                                // After a low phase (or a new trigger press), play a random high volume phase
    volumePhase = 1;
    float currentRMS = 0;
    if (outputTime > 0) currentRMS = sqrt(outputSquareSum / outputTime);

    int middleHV = (HVmin + HVmax) / 2;
    if (currentRMS > RMSout) {                                            // Above RMSout: use the quieter half of the high range
      volumeLevel = random(HVmin, middleHV + 1);
    } else {                                                              // At or below RMSout: use the louder half of the high range
      volumeLevel = random(middleHV, HVmax + 1);
    }
    volumePhaseDuration = random(minimumHVDuration, maximumHVDuration + 1);
  }

  volumePhaseStart = time;
  analogWrite(volumePin, volumeLevel);
}
// ======================================================= END ===================================================

#undef setup
#undef loop
#undef digitalRead
#undef analogWrite
#undef tone
#undef noTone

// ----------------------------------------------------Test Bench------------------------------------------------------------

unsigned long lastPlot = 0;

bool hearingRested = true;
bool hearingHigh = false;
unsigned long hearingHighStart = 0;
float hearingHoldValue = 100;                   // Sensitivity held for the first hearingResetDelay ms of a high interval
unsigned long recoveryStart = 0;

int whiteLevels[numWhitePins];                  // Last value written to each white LED pin
bool whiteLit = false;
unsigned long whiteLitSince = 0;
unsigned long whiteDarkSince = 0;
bool lightOn = false;
unsigned long lightOnSince = 0;

bool pupilConstricting = false;
unsigned long pupilResponseStart = 0;
float pupilStartDiameter = pupilBaseline;
const float pupilConstricted = pupilBaseline - 0.63 * log10(ledLux / 400.0);   // Fully constricted diameter in mm


void setup() {                                  // Start the plotter's serial port, then the product's own setup
  Serial.begin(115200);
  productSetup();
}

void loop() {                                   // Run the product
  productLoop();
  benchUpdate();
}


int benchDigitalRead(int pin) {                 // Every trigger check in the product also updates the test bench
  benchUpdate();
  if (pin != triggerPin) return digitalRead(pin);
  return (millis() < simTriggerStart) ? LOW : HIGH;                  // Simulated trigger
}

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

void benchTone(int, int) {}                     // The speaker stays silent
void benchNoTone(int) {}


void benchUpdate() {                            // Track the models and print a point every plotIntervals[graph - 1] ms
  unsigned long now = millis();
  trackHearing(now);
  trackLight(now);

  if (now - lastPlot < plotIntervals[graph - 1]) return;
  lastPlot = now;
  readGraphChoice();
  plot(now);
}

void readGraphChoice() {                        // Send 1-4 from the Serial Plotter to switch graphs
  while (Serial.available() > 0) {
    char c = Serial.read();
    if (c >= '1' && c <= '4') graph = c - '0';
  }
}


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

    case 3:                                                  // Hearing sensitivity over the high and low volume intervals
      printValue("Hearing_Sensitivity", hearingSensitivity(now), false);
      printValue("High_Volume", (volumePhase == 1) ? 100 : 0, true);
      break;

    case 4:                                                  // Pupil diameter
      printValue("Pupil_mm", pupilDiameter(now), false);
      printValue("Light_Target_mm", lightOn ? pupilConstricted : pupilBaseline, true);
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
