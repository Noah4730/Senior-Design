#pragma once

// Swaps the product's hardware calls for the test bench versions

int benchDigitalRead(int pin);
void benchAnalogWrite(int pin, int value);
void benchTone(int pin, int frequency);
void benchNoTone(int pin);
int benchAnalogRead(int pin);

#define digitalRead benchDigitalRead
#define analogRead benchAnalogRead
#define analogWrite benchAnalogWrite
#define tone benchTone
#define noTone benchNoTone
