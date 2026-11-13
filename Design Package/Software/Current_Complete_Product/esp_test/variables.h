#pragma once

#include <Arduino.h>


// ---------------- Test bench setup ----------------

// The test bench runs alongside the real hardware: every pin write still reaches the LEDs and speaker,
// and the models (RMS, dB, hearing, pupil) follow those same writes. Plotting only runs while the trigger is held.
const bool simulateTrigger = false;             // true = ignore the trigger pin and use simTriggerStart/simTriggerEnd
const bool simulateMic = false;                 // true = ignore the mic pin and use the simulated room below

const unsigned long simTriggerStart = 2000;
const unsigned long simTriggerEnd = 0;          // ms when the simulated trigger is released (0 = never)

inline int graph = 1;                           // 1 = speaker RMS, 2 = average dB, 3 = hearing sensitivity, 4 = pupil, 5 = sound limit
const unsigned long plotIntervals[] = {200, 200, 50, 100, 50};   // ms between points for graphs 1-5

const float simQuietRoomDb = -6.0;              // Mic reading offset from the speaker dB in a normal room
const float simLoudRoomDb = 6.0;                // Mic reading offset while the room is loud (e.g. aimed at a nearby wall)
const unsigned long simLoudStart = 12000;       // ms when the room turns loud
const unsigned long simLoudEnd = 22000;         // ms when the room is back to normal
const float simMicNoiseDb = 1.0;                // Random +/- dB added to each simulated mic sample

const float LVminDb = 40.0;                     // Low volume min/max db
const float LVmaxDb = 80.0;
const float maxSpeakerDb = 130.0;
const float exposureLimitDb = 100.0;            // Exposure time is tracked above this level
const float speakerOffDb = 30.0;                // dB plotted while the speaker is off

const unsigned long hearingResetDelay = 100;    // ms of high volume before hearing sensitivity resets to 0

const float ledLux = 100000.0;                   // Lux at the eye while the white LEDs are on
const unsigned long pupilDelay = 200;           // ms of white light before the pupil starts constricting
const float pupilBaseline = 3.21;


// ==================================== Current_Complete_Product.ino ==========================================
#include <SPI.h>
#include <SD.h>

/*
  Christina Walker: safety checks and data logging functions
  Noah Schatz: light and sound control functions
*/

// const int chipSelect = 10;
// File logFile;

// const int greenPins[] = {16, 17, 18, 19, 21};
// Arduino uses GPIO numbers, not module pin numbers. Module pins are noted on each line.
const int greenPins[] = {32, 23, 22, 21, 19};     // module pins 8, 37, 36, 33, 31
// const int whitePins[] = {22, 23, 25, 26, 27};
const int whitePins[] = {18, 17, 16, 14, 13};     // module pins 30, 28, 27, 13, 16
const int numGreenPins = 5;
const int numWhitePins = 5;
// const int speakerPin = 32;
const int speakerPin = 25;                        // module pin 10
// const int volumePin = 33;
const int volumePin = 26;                         // module pin 11
// const int triggerPin = 34;
const int triggerPin = 34;                        // module pin 6 (input only)
// const int soundSensorPin = 35;
const int soundSensorPin = 35;                    // module pin 7 (input only, ADC1)
const int LVmin = 20;
const int LVmax = 80;
const int HVmin = 150;
const int HVmax = 255;
const int RMSout = 160;
const int minimumOffDuration = 250;                   // Speaker off interval between high volume phases
const int maximumOffDuration = 750;
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
inline int greenBrightness = 255;
inline int whiteBrightness = 255;
inline int flickerMode = 1;                            // 0 = all at once, 1 = star, 2 = two at a time, 3 = three at a time
inline int whiteStep = 0;                              // Current flash in the white flicker pattern
inline int rotSpeed = 28;
inline int speakerFreq = 1000;
inline int sweep = 50;
inline int volumeLevel = 0;
inline int volumePhase = -1;                           // -1 = trigger off, 0 = speaker off interval, 1 = high volume
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

// Christina Walker - sound safety controller (safety_CW.cpp)
inline int safetyGain= 255;
// const int micPin= 34;       // analog input pin the microphone is wired to
const int micPin= soundSensorPin;   // same mic as soundSensorPin (GPIO35, module pin 7)

//safety limit placeholders
// if measured sound goes above this level, speaker output should be reduced
// const float soundLimitDb= 130.0;   // immediate threshold (placeholder)

//exposure time is tracked when the sound is at/above this dB level
// const float exposureLimitDb= 110.0;  //lower threshold where exposure time is recorded (placeholder)

//maximum accumulated exposure time at/above exposureLimitDb
//currently placeholder and will be replaced with approved value
const unsigned long maxExposureTime= 30000UL;  //30 seconds- max cumulative time allowed at/above exposureLimitDb before shutdown (placeholder)

//timing

inline unsigned long lastSampleTime= 0;  //timestamp (ms) of the last microphone reading. used for the 50ms sample timer

//how often microphone is checked
const unsigned long sampleInterval= 50;  //sampling period (ms). Also the time slice added to exposureTime each loud sample

inline unsigned long lastExposureTime= 0;  //timestamp used to calculate actual elapsed exposure time

//counts consecutive readings at/above the exposure threshold
inline int highReadingCount= 0;
const int requiredHighReadings= 3;  //number of consecutive high readings required before exposure is accumulated

//tracks whether the immediate sound-limit event has already been logged
inline bool soundLimitEventLogged= false;

//total exposure time
inline unsigned long exposureTime= 0;  //running total (ms) of time spent at/above exposureLimitDb since the system was last activated

//system state
inline bool safetyShutdown= false; //latches TRUE once maxExposureTime is exceeded. Stays true (keeps output reduced) until trigger goes LOW

//speaker volume control
const int fullGain= 255;
const int safetyGainStep= 5;

// Christina Walker - battery indicator (battery_CW.cpp)
const int batteryPin=36; //battery voltage input: GPIO36 / SENSOR_VP, module pin 4 (ADC1, input only) - placeholder, confirm wiring
// const int buzzerPin=33;  //low battery buzzer (ESP32 GPIO number- placeholder pin)
const int buzzerPin=33;  //low battery buzzer: GPIO33, module pin 9 - placeholder, confirm wiring

// scaling from the divider
const float dividerRatio=(100000.0+12000.0)/12000.0;

// battery voltage that counts as 20% left (full is about 21 V, nominal about 18.5 V)
// const float lowBatteryVoltage=17.5;
const float lowBatteryVoltage=16.5;

// the reading is checked once a second, and the warning only starts after several low
// readings in a row, so one dip (for example when the speaker draws power) does not trigger it
inline unsigned long lastCheck=0;                  //timestamp (ms) of the last battery check
const unsigned long checkInterval=1000;     //time (ms) between battery checks
inline int lowReadingCount=0;                      //how many checks in a row were below the threshold
const int requiredLowReadings=5;            //low readings in a row needed before warning

inline unsigned long lastWarning=0;          //timestamp (ms) of the last time we sounded the buzzer
const unsigned long warningInterval=5000;   //minimum gap (ms) between buzzer warnings= 5 seconds



// ----------------------------------------------------Test Bench------------------------------------------------------------

inline unsigned long lastPlot = 0;

inline bool hearingRested = true;
inline bool hearingHigh = false;
inline unsigned long hearingHighStart = 0;
inline float hearingHoldValue = 100;                   // Sensitivity held for the first hearingResetDelay ms of a high interval
inline unsigned long recoveryStart = 0;

inline int whiteLevels[numWhitePins];                  // Last value written to each white LED pin
inline bool whiteLit = false;
inline unsigned long whiteLitSince = 0;
inline unsigned long whiteDarkSince = 0;
inline bool lightOn = false;
inline unsigned long lightOnSince = 0;

inline bool pupilConstricting = false;
inline unsigned long pupilResponseStart = 0;
inline float pupilStartDiameter = pupilBaseline;
const float pupilConstricted = pupilBaseline - 0.63 * log10(ledLux / 400.0);   // Fully constricted diameter in mm
