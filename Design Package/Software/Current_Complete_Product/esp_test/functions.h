#pragma once

// Function prototypes shared by every .cpp file

int benchDigitalRead(int pin);
void benchAnalogWrite(int pin, int value);
void benchTone(int pin, int frequency);
void benchNoTone(int pin);
int benchAnalogRead(int pin);
float volumeToDb(float level);

// esp_test.ino - the product's setup() and loop()
void productSetup();
void productLoop();

void logEvent();
void runGreen();
void sysTime();
void flicker();

// logging_CW.cpp - logging functions
void logEvent(const char* type, int pin, const char* stateStr);

// green_light_NJS.cpp - green light functions
void runGreen();

// white_light_NJS.cpp - white light functions
void sysTime(unsigned long ms);
void setWhite(int brightness);
void showWhiteStep();
void flicker(int duration);

// speaker_NJS.cpp - speaker functions
void speaker();
void volume();

// hysteresis_NJS.cpp - sound limit functions
void soundLimiter(int raw);
void checkSoundLimit(float dB);

// safety_CW.cpp - sound safety controller
void safetyLoop(float dB);
void logSafetyEvent(float dB, const char* event);
void reduceSpeakerOutput();
void restoreSpeakerOutput();

// battery_CW.cpp - battery indicator (low battery chirp)
void batterySetup();    // the setup() of the battery indicator
void batteryLoop();     // the loop() of the battery indicator, call it every pass

// testbench_core_NJS.cpp - simulated trigger and Serial Plotter output
int benchTrigger();
void benchUpdate();
void readGraphChoice();
void plot(unsigned long now);
void printValue(const char* label, float value, bool last);

// speaker_safety_testbench_NJS.cpp - speaker safety test bench (simulated mic, RMS, dB)
float runningRms(unsigned long now);

// hearing_testbench_NJS.cpp - hearing sensitivity test bench
float hearingRecovery(float t);
float hearingSensitivity(unsigned long now);
void trackHearing(unsigned long now);

// pupil_testbench_NJS.cpp - pupil test bench
void trackLight(unsigned long now);
float pupilConstriction(float t);
float pupilRecovery(float t);
float pupilDiameter(unsigned long now);
void startPupilResponse(bool constricting, unsigned long time);
