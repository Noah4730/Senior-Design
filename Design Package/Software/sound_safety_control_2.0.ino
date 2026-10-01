const int micPin= A2;        // analog input pin the microphone is wired to
const int triggerPin= A0;    // digital input- HIGH means sound system is running

//safety limit placeholders
// if measured sound goes above this level, speaker output should be reduced
const float soundLimitDb= 130.0;   // immediate threshold (placeholder)

//exposure time is tracked when the sound is at/above this dB level
const float exposureLimitDb= 110.0;  //lower threshold where exposure time is recorded (placeholder)

//maximum accumulated exposure time at/above exposureLimitDb
//currently placeholder and will be replaced with approved value
const unsigned long maxExposureTime= 30000UL;  //30 seconds- max time allowed at/above exposureLimitDb before shutdown (placeholder)

//timing

unsigned long lastSampleTime= 0;  //timestamp (ms) of the last microphone reading. used for the 50ms sample timer

//how often microphone is checked
const unsigned long sampleInterval= 50;  //sampling period (ms). Also the time slice added to exposureTime each loud sample

unsigned long lastExposureTime= 0;  //timestamp used to calculate actual elapsed exposure time

//counts consecutive readings at/above the exposure threshold
int highReadingCount= 0;
const int requiredHighReadings= 3;  //number of consecutive high readings required before exposure is accumulated

//tracks whether the immediate sound-limit event has already been logged
bool soundLimitEventLogged= false;

//total exposure time
unsigned long exposureTime= 0;  //running total (ms) of time spent at/above exposureLimitDb since the system was last activated

//system state
bool safetyShutdown= false; //latches TRUE once maxExposureTime is exceeded. Stays true (keeps output reduced) until trigger goes LOW

void setup() {

  Serial.begin(9600);   //serial open aka dB readings/ warnings

  pinMode(micPin, INPUT);   //microphone input pin
  pinMode(triggerPin, INPUT);  // trigger/system active input pin
}

//main

void loop() {
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
      int sensorValue= analogRead(micPin);  // raw analog mic reading, 0-1023

      float dB= 30.0+(sensorValue/1023.0)*104.0;

      //display reading
      Serial.print("Raw: ");
      Serial.print(sensorValue);

      Serial.print("   dB: ");
      Serial.println(dB);

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
      // Cumulative check: if this reading is at/above the (lower)
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
  /*
    placeholder-arduino will tell amplifier
    or audio control circuit to reduce/stop the speaker
    output
    exact code cannot be written yet because the
    amplifier/control circuit has not been finalized
  */
  Serial.println("Reduce Speaker Output");   // temp for the real hardware command, once the amp/control circuit is finalized
}
//CW



