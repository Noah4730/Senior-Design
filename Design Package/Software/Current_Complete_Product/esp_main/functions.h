#pragma once

// Function prototypes shared by every .cpp file

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
