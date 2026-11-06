#pragma once

#include <Arduino.h>
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
const float soundLimitDb = 130.0;
const float soundReleaseDb = 105.0;
const int requiredQuietReadings = 20;
const unsigned long micWindow = 50;


const int whitePatternSteps = 5;
const int whitePatterns[3][whitePatternSteps][3] = {         // White flicker patterns for modes 1-3
  {{1, 0, 0}, {3, 0, 0}, {5, 0, 0}, {2, 0, 0}, {4, 0, 0}},   // star, one pin at a time
  {{1, 2, 0}, {4, 3, 0}, {5, 1, 0}, {2, 3, 0}, {5, 4, 0}},   // two pins at a time
  {{1, 2, 3}, {3, 4, 5}, {5, 1, 2}, {2, 3, 4}, {4, 5, 1}}    // three pins at a time
};

inline int freqWhite = 40;
inline int freqGreen = 35;
inline int greenBrightness = 255;                      // Brightness for all green pins (0-255)
inline int whiteBrightness = 255;                      // Brightness for all white pins (0-255)
inline int flickerMode = 1;                            // 0 = all at once, 1 = star, 2 = two at a time, 3 = three at a time
inline int whiteStep = 0;                              // Current flash in the white flicker pattern
inline int rotSpeed = 28;
inline int speakerFreq = 1000;
inline int sweep = 50;
inline int volumeLevel = LVmin;
inline int volumePhase = -1;                           // -1 = off, 0 = low volume, 1 = high volume
inline int state = 0;
inline int greenIndex = 0;
inline unsigned long lastTime1 = 0;
inline unsigned long lastTime2 = 0;
inline unsigned long lastTime3 = 0;
inline unsigned long volumePhaseStart = 0;
inline unsigned long volumePhaseDuration = 0;
inline unsigned long outputTime = 0;                   // Total ms counted in the running RMS
inline float outputSquareSum = 0;                      // Sum of volumeLevel^2 * ms counted in the running RMS
inline bool soundLimited = false;                      // true = high volume held to the quieter half of the high range
inline int quietReadings = 0;                          // Consecutive high volume readings at or below soundReleaseDb
inline float micReadingDb = 0;
inline unsigned long lastMicWindow = 0;
inline int micMin = 4095;
inline int micMax = 0;
inline long micSum = 0;
inline int micCount = 0;                               // Number of mic samples in the current window
