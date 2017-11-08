/* Takes UV reading from sensor every 1 minute.
 * Summation counter keeps tab of total UV exposure each day, so add newest reading to counter.
 * Lights will increase in number and intensity as total UV exposure (summation counter) increases.
 * Alarm will sound when total exposure reaches certain max threshold.
 * Right button will mute alarm and assume more sunscreen was applied. Will only increase UV counter summation by 5% for next 2 hours
 * because assumes suncreen blocks 95% of UV light.
 * If you don't press button in the alarm time (30 seconds), assumes you didn't apply sunscreen. Will increase UV counter by full amount
 * for next 2 hours.
 * Alarm will sound every 2 hours if alarm hasn't already sounded because of .
 * Need a way to not increase UV counter and delay 2 hour alarm if indoors (maybe if UV sensing is below a certain amount, so thinks 
 * you are indoors).
 * Reset button will reset counter for each new day or if device is worn by a new person in the middle of the day.
 * Each day, summation counter resets if date and time works on playground.
 * Left button will be used to make skin color measurement.
 * Will utilize accelerometer in order to beep the person sooner if s/he was exercising and sweating.
 * Parts based on Circuit Playground Analog Sensor Demo
 */

#include <Adafruit_CircuitPlayground.h>
#include <Wire.h>
#include <SPI.h> // from analog sensor demo code
//#include "Adafruit_SI1145.h" // will add when we have UV sensor


// Used for blinking function
// constants won't change. Used here to set a pin number :
const int ledPin =  13;      // the number of the LED pin

// Variables will change :
int ledState = LOW;             // ledState used to set the LED

// Generally, you should use "unsigned long" for variables that hold time
// The value will quickly become too large for an int to store
unsigned long previousMillis = 0;        // will store last time LED was updated

// constants won't change :
const long interval = 100;           // interval at which to blink (milliseconds)
// end of blinking function variables



// Change the analog input value below to try different sensors:
#define ANALOG_INPUT  A5  // Specify the analog input to read.
                          // Circuit Playground has the following
                          // inputs available:
                          //  - A0  = temperature sensor / thermistor
                          //  - A4  = sound sensor / microphone
                          //  - A5  = light sensor
                          //  - A7  = pin #6 on board
                          //  - A9  = pin #9 on board
                          //  - A10 = pin #10 on board
                          //  - A11 = pin #12 on board

// These defines set the range of expected analog values.
// This is used to scale the NeoPixels, sound, etc.
#define VALUE_MIN     0
#define VALUE_MAX     200

// These defines set the range of pixel color when mapping
// to the sensor value.
#define COLOR_RED_MIN    255  
#define COLOR_GREEN_MIN  0
#define COLOR_BLUE_MIN   0

#define COLOR_RED_MAX    255
#define COLOR_GREEN_MAX  255
#define COLOR_BLUE_MAX   0

// These defines set the range of sound frequencies when
// mapping to the sensor value.
#define TONE_FREQ_MIN    523  // C5 note
#define TONE_FREQ_MAX    988  // B5 note

uint32_t elapsedtime;
uint32_t currenttime;
uint32_t reapplytime;
uint16_t UVsum = 0;

void setup() {
  Serial.begin(9600); // Setup serial port.
  Serial.println("Circuit Playground UV/photodiode sensor!");
    CircuitPlayground.begin(); // Setup Circuit Playground library.
}

void loop() {
  // Get the sensor value and print it out (can use serial plotter
  // to view realtime graph!)
  uint16_t value = analogRead(ANALOG_INPUT);
  Serial.println(value, DEC);
  UVsum = UVsum + value;
  Serial.println(UVsum, DEC);

  if(UVsum > thresholdsum){
    playalarm(); 
  }
  delay (120000); // wait 2 minutes (120,000 ms) for next reading
  
  // Map the sensor value to a color.
  // Use the range of minimum and maximum sensor values and
  // min/max colors to do the mapping.
  int red = map(value, VALUE_MIN, VALUE_MAX, COLOR_RED_MIN, COLOR_RED_MAX);
  int green = map(value, VALUE_MIN, VALUE_MAX, COLOR_GREEN_MIN, COLOR_GREEN_MAX);
  int blue = map(value, VALUE_MIN, VALUE_MAX, COLOR_BLUE_MIN, COLOR_BLUE_MAX);

  // Light up pixel #4 and 5 with the color.
  CircuitPlayground.clearPixels();
  CircuitPlayground.setPixelColor(1, red, green, blue);
  CircuitPlayground.setPixelColor(3, red, green, blue);
  CircuitPlayground.setPixelColor(5, red, green, blue);
  CircuitPlayground.setPixelColor(6, red, green, blue);
  CircuitPlayground.setPixelColor(7, red, green, blue);
  CircuitPlayground.setPixelColor(8, red, green, blue);
  CircuitPlayground.setPixelColor(9, red, green, blue);

  // Map the sensor value to a tone frequency.
  int frequency = map(value, VALUE_MIN, VALUE_MAX, TONE_FREQ_MIN, TONE_FREQ_MAX);

  // Play the tone if the slide switch is turned on (to the left).
  if (CircuitPlayground.slideSwitch()) {
    // Play tone of the mapped frequency value for 100 milliseconds.
    CircuitPlayground.playTone(frequency, 100);
  }

  // Delay for a bit and repeat the loop.
  delay(1000);
}

void playalarm(){ 
  //code to play non-annoying alarm and light up lights
unsigned long currentMillis = millis(); //current time
while(blinkcounter < 10) { // only play alarm for 5 seconds)
  currentMillis = millis(); // refresh current time
  if (currenttime - previousMillis >= interval) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;
    // if the LED is off turn it on and vice-versa:
    if (ledState == LOW) {
      ledState = HIGH;
    } else {
      ledState = LOW;
    }
    blinkcounter++;
    // set the LED with the ledState of the variable:
    digitalWrite(ledPin, ledState);
}
} //end of while loop for blinking
} //end of play alarm
