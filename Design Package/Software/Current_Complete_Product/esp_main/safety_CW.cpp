// Christina Walker - sound safety controller
#include "variables.h"
#include "functions.h"

/*
void safetySetup() {

  // Serial.begin(9600);   //serial open aka dB readings/ warnings

  pinMode(micPin, INPUT);   //microphone input pin
  // analogReadResolution(10);   // ESP32 reads 0-4095 by default. This makes it 0-1023 so the dB math below still works
}
*/

//main

// void safetyLoop() {
void safetyLoop(float dB) {
  unsigned long currentTime= millis();   //get current elapsed time once per loop

  //new measurement every 50 ms
  //non-blocking timer: takes new reading once sampleInterval (50ms)
  //has elapsed since last one, instead of sampling as fast as
  // the loop can go (which would flood serial and make exposureTime
  // inaccurate/unpredictable).
  if (currentTime-lastSampleTime >= sampleInterval){

    lastSampleTime= currentTime;  //reset sample timer

    //check whether sound system is active or not
    // monitor safety limits while sound system is
    // actually running (trigger HIGH). If off, skip to
    // else branch below to reset state for next time
    if (digitalRead(triggerPin)== HIGH){

      //calculate the actual time since the previous safety reading
      unsigned long elapsedTime= currentTime-lastExposureTime;
      lastExposureTime= currentTime;

      //read microphone
      // int sensorValue= analogRead(micPin);  // raw analog mic reading, 0-1023

      // float dB= 30.0+(sensorValue/1023.0)*104.0;
      // float dB= 30.0+(sensorValue/4095.0)*104.0;

      //display reading
      // Serial.print("Raw: ");
      // Serial.print(sensorValue);
      //
      // Serial.print("   dB: ");
      // Serial.println(dB);

      //sound too loud?
      // immediate..00000000000000000000 if initial reading is already at/ above
      // high sound threshold, immediately reduce output
      if (dB >= soundLimitDb){

        //log the safety event only once when the sound first reaches the limit
        if (!soundLimitEventLogged){
          logSafetyEvent(dB, "Sound Limit Exceeded");
          soundLimitEventLogged= true;
        }

        //Serial.println("Warning:Sound level too high.");

        reduceSpeakerOutput();
      }
      else {
        //allow a new sound-limit event to be logged if the level drops below the limit
        soundLimitEventLogged= false;
      }

      //safety check-exposure time
      // if this reading is at/above the (lower)
      // exposure threshold, count consecutive high readings before
      // adding the actual elapsed time to the running exposureTime total.
      if (dB >= exposureLimitDb){

        highReadingCount++;

        //only accumulate exposure after the sound has remained
        //at/above the exposure threshold for the required number of readings
        if (highReadingCount >= requiredHighReadings){

          exposureTime+= elapsedTime;

          Serial.print("Exposure time: ");
          Serial.print(exposureTime);
          Serial.println(" ms");

          // check if allowed exposure been reached
          // when accumulated time at/above exposureLimitDb crosses
          // allowed maximum, permanently reduce output and latch
          // safetyShutdown so it stays reduced even if the level later dips
          // back down below the thresholds.

          if (exposureTime>= maxExposureTime){
            Serial.println("Warning: Maximum exposure reached.");
            logSafetyEvent(dB, "Maximum Exposure Reached");
            reduceSpeakerOutput();
            safetyShutdown= true;
          }
        }
      }
      else {
        //reset consecutive high-reading count when sound drops below exposure threshold
        highReadingCount= 0;
      }

      //if safety shutdown happened, keep output reduced until system is restarted
      // reset according to the final design
      // even on occasions where dB has dropped
      // back below both thresholds, continues giving "reduce output"
      // command every sample as long as safetyShutdown is latched as true

      if (safetyShutdown){
        reduceSpeakerOutput();
      }
    }
    //sound system no longer active
    else {
      //reset exposure for next activation
      // trigger goes LOW (turned off). clears accumulated
      // exposure time and unlatches safetyShutdown. Next time
      // system is triggered HIGH again it starts over fresh.

      exposureTime= 0;
      safetyShutdown= false;
      highReadingCount= 0;
      soundLimitEventLogged= false;
      lastExposureTime= currentTime;
      restoreSpeakerOutput();   //put the speaker output back to full for next activation
    }
  }
}

// safety event log
void logSafetyEvent(float dB, const char* event) {
  Serial.print("Safety Event - Time: ");
  Serial.print(millis());
  Serial.print(" ms, dB: ");
  Serial.print(dB);
  Serial.print(", ACTION: ");
  Serial.println(event);
}

// reduce speaker output

void reduceSpeakerOutput() {
  if (safetyGain > 0) {
    safetyGain-= safetyGainStep;
    if (safetyGain < 0) safetyGain= 0;
  }
  Serial.println("Reduce Speaker Output");
}

// restore speaker output
// called when the trigger goes LOW, puts the amp back to full output

void restoreSpeakerOutput() {
  safetyGain= fullGain;
}
