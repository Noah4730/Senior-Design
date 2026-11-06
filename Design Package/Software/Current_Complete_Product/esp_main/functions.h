#pragma once

// Function prototypes shared by every .cpp file

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
