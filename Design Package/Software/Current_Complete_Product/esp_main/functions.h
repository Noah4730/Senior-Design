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
