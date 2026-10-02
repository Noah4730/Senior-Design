//battery indicator for Zeus
// low battery chirp warning
// periodically check battery voltage, and if it's dropped
//  below a low battery threshold, beep every few seconds as a warning
//  work in progress
//    1) lowBatteryVoltage is hardcoded to 0.0 as a placeholder- real
//       voltage that corresponds to "20% battery" still needs to be
//       measured/calculated and filled in.
//    2) batteryVoltage inside loop() is also hardcoded to 0.0 as a
//       placeholder-no actual analogRead(batteryPin) happening
//       yet to get a real live voltage reading, so as written, the low
//       battery check is comparing 0.0< 0.0, which is always false and
//       will never actually trigger chirp warning

const int batteryPin=A2; //battery voltage input
const int buzzerPin=7;   //low battery buzzer

//still need to figure out the actual voltage for 20%
const float lowBatteryVoltage=0.0;   //placeholder threshold-real 20% battery voltage TBD

unsigned long lastWarning= 0;  //timestamp (ms) of the last time we sounded the buzzer
const unsigned long warningInterval= 5000;   //minimum gap (ms) between buzzer warnings= 5 seconds
// set up runs once at power on/ reset

void setup(){
  pinMode(batteryPin, INPUT);    // set up battery voltage sense pin as input
  pinMode(buzzerPin, OUTPUT);    //set up the buzzer pin as output (tone())

  Serial.begin(9600);            // status messages can print (like low battery)
}

//main
//  every pass, check current battery voltage against low battery threshold
//  if  below the threshold and warningInterval
//  ms have passed since last beep, sound short warning tone and log
//  Low battery to serial. This rate limiting (lastWarning/millis())
//  is what keeps the buzzer from beeping non stop once battery is low/
//  it only chirps once every 5 seconds

void loop(){

  //placeholder for battery voltage reading
  // replace this with a real reading, something like
  // float batteryVoltage= analogRead(batteryPin) * (referenceVoltage / 1023.0);
  // once actual sensing/scaling for this board is worked out.

  float batteryVoltage= 0.0;

  if (batteryVoltage<lowBatteryVoltage){   //is battery below the low battery threshold?

    //short warning every few seconds
    // only  beep if enough time (warningInterval) has passed since
    // last beep. Non-blocking timer pattern.
    if (millis()-lastWarning >= warningInterval){
      tone(buzzerPin, 2000, 200);   //2000Hz tone on buzzer for 200ms
      lastWarning=millis();    //reset cooldown timer
      Serial.println("Low battery"); //log warning to serial 
    }
  }
}


//CW
