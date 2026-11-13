//battery indicator for Zeus
//
// (placeholders):
//    dividerRatio: battery volts divided by the volts at the sense pin.
//    lowBatteryVoltage: the battery voltage that counts as 20% left (pack is
//       5 lithium-ion cells, about 21 V full, about 18.5 V nominal). Set to 16.5 V


#include "variables.h"
#include "functions.h"

// void setup(){
void batterySetup(){
  pinMode(batteryPin, INPUT);    //set up the battery voltage sense pin as an input
  pinMode(buzzerPin, OUTPUT);    //set up the buzzer pin as an output (driven via tone())

  // Serial.begin(9600);
}

// void loop(){
void batteryLoop(){

  if (millis()-lastCheck >= checkInterval){   //time for the next battery check?
    lastCheck=millis();

    //8 quick readings to smooth out noise. analogReadMilliVolts gives the
    // voltage at the pin in millivolts (the ESP32 corrects for its own ADC error).
    // float pinMillivolts=0.0;
    // for (int i=0; i<8; i++){
    float pinMillivolts=analogReadMilliVolts(batteryPin);
    // for (int i=1; i<8; i++){
    for (int i=1; i<3; i++){  
      pinMillivolts+=analogReadMilliVolts(batteryPin);
    }
   
    pinMillivolts/=3.0;   

    //voltage at pin, scaled back up to the real battery voltage
    float batteryVoltage=(pinMillivolts/1000.0)*dividerRatio;

    Serial.print("Battery: ");
    Serial.print(batteryVoltage);
    Serial.println(" V");

    if (batteryVoltage<lowBatteryVoltage){   //is battery below the low battery threshold?
      lowReadingCount++;
    }
    else {
      lowReadingCount=0;                     //battery looks fine, start count over
    }
  }

  if (lowReadingCount>=requiredLowReadings){

    //short warning every few seconds
    //non-blocking "cooldown" timer pattern.
    if (millis()-lastWarning >= warningInterval){
      tone(buzzerPin, 2000, 200);   //play a 2000Hz tone on the buzzer for 200ms
      lastWarning=millis();          //reset the cooldown timer
      Serial.println("Low battery"); //log the warning 
    }
  }
}
