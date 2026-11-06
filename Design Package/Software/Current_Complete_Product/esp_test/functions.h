#pragma once

// Function prototypes shared by every .cpp file

int benchDigitalRead(int pin);
void benchAnalogWrite(int pin, int value);
void benchTone(int pin, int frequency);
void benchNoTone(int pin);
int benchAnalogRead(int pin);
float volumeToDb(float level);

// esp_test.ino - the product's setup() and loop(), renamed for the test bench
void productSetup();
void productLoop();

void logEvent();
void runGreen();
void sysTime();
void flicker();

// logging.cpp - logging functions
void logEvent(const char* type, int pin, const char* stateStr);

// green_light.cpp - green light functions
void runGreen();

// white_light.cpp - white light functions
void sysTime(unsigned long ms);
void setWhite(int brightness);
void showWhiteStep();
void flicker(int duration);

// speaker.cpp - speaker functions
void speaker();
void volume();

// hysteresis.cpp - sound limit functions
void soundLimiter(int raw);
void checkSoundLimit(float dB);

// testbench_core.cpp - simulated trigger and Serial Plotter output
void benchUpdate();
void readGraphChoice();
void plot(unsigned long now);
void printValue(const char* label, float value, bool last);

// speaker_safety_testbench.cpp - speaker safety test bench (simulated mic, RMS, dB)
float runningRms(unsigned long now);

// hearing_testbench.cpp - hearing sensitivity test bench
float hearingRecovery(float t);
float hearingSensitivity(unsigned long now);
void trackHearing(unsigned long now);

// pupil_testbench.cpp - pupil test bench
void trackLight(unsigned long now);
float pupilConstriction(float t);
float pupilRecovery(float t);
float pupilDiameter(unsigned long now);
void startPupilResponse(bool constricting, unsigned long time);
