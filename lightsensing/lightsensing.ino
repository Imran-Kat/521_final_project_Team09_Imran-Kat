/* Takes UV reading from sensor every 1 or 2 minute(s).
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
 * Sections of  code based on Circuit Playground Analog Sensor Demo and blink without delay code from 
 * for learning https://programmingelectronics.com/using-the-print-function-with-arduino-part-1/
 */

#include <Adafruit_CircuitPlayground.h>
#include <Wire.h>
#include <SPI.h> // from analog sensor demo code
#include "Adafruit_SI1145.h" // will add when we have UV senisor

//Song constants
#define  c        261.626    // 261 Hz
#define  d        293.665    // 294 Hz
#define  e        329.628    // 329 Hz
#define  f        349.228    // 349 Hz
#define  g        391.995    // 392 Hz
#define  a        440.000    // 440 Hz
#define  b        493.883    // 493 Hz
#define  C        523.251    // 523 Hz
#define  R        0

int melody[] = {  g, c, d, e, e, R, 
                  e, d, e, c, c, R,
                  c, d, e, f, a, R,
                  a, g, f, e, R
};
int noteDurations[] = { 4, 4, 4, 4, 4, 8,
                        4, 4, 4, 4, 4, 8,
                        4, 4, 4, 4, 4, 8,
                        4, 4, 4, 4, 8
                      };
//end of song constants

Adafruit_SI1145 uv = Adafruit_SI1145();

// Used for blinking function
// constants won't change. Used here to set a pin number :
const int ledPin =  9;      // the number of the LED pin

// Variables will change :
int ledState = LOW;             // ledState used to set the LED

// Generally, you should use "unsigned long" for variables that hold time
// The value will quickly become too large for an int to store
unsigned long previousMillis = 0;        // will store last time LED was updated

// constants won't change :
const long interval = 300;           // interval at which to blink (milliseconds)
// end of blinking function variables
unsigned long currentMillis = 0;


// Change the analog input value below to try different sensors:
#define ANALOG_INPUT  A5  // Specify the analog input to read.
                          // Circuit Playground has the following
                          // inputs available:
                          //  - A0  = temperature sensor / thermistor
                          //  - A4  = sound sensor / microphone
                          //  - A5  = light sensor


uint32_t elapsedtime;
uint32_t currenttime;
uint32_t reapplytime;
uint16_t UV = 0; // might need to be float, converted value of UV index into J/m^2 (how much energy has hit your screen over the time of the interval)
uint16_t UVsum = 0;
uint16_t blinkcounter = 0;
uint16_t threshold = 500; // Threshold, in units of J/m^2 (probably around 300 for light skinned people), until warning alarm sounds and lights blink

int cycles = 0; //used in alarm function to count number of times leds blink

bool leftButtonPressed;
bool rightButtonPressed;

int sensorinterval = 5000; // time interval between UV readings - e.g. 120,000 = every 2 minutes

void setup() {
  Serial.begin(9600); // Setup serial port.
  Serial.begin(115200);
  Serial.println("Circuit Playground UV/photodiode sensor!");
  CircuitPlayground.begin(); // Setup Circuit Playground library.
}

void loop() {
  leftButtonPressed = CircuitPlayground.leftButton();
  rightButtonPressed = CircuitPlayground.rightButton();

  if(leftButtonPressed){
    Serial.println("*****************Left button!");
    CircuitPlayground.playTone(262,200);
  }

  if(rightButtonPressed){
    Serial.println("**********************************************************Right button!");
    CircuitPlayground.playTone(292,100);
    //showmethelight();
  }
  
  // Get the sensor value and print it out
  Serial.println("Circuit Playground UV/photodiode sensor!");
  
  //Temp values, not using UV reading from sensor
  uint16_t value = analogRead(ANALOG_INPUT);
  UVsum = UVsum + value;
  Serial.print("current value: ");
  Serial.println(value, DEC);
  Serial.print("Sum of UV value since last reset: ");
  Serial.println(UVsum, DEC);

  Serial.println("===================");
  Serial.print("Vis (sensor): "); Serial.println(uv.readVisible()); //from UV sensor
  Serial.print("IR (sensor): "); Serial.println(uv.readIR()); // from UV sensor

  float UVindex = uv.readUV();
  // the index is multiplied by 100 so to get the
  // integer index, divide by 100!
  UVindex /= 100.0;  
  Serial.print("UV: ");  Serial.println(UVindex);
  //end of Temp values

/*// uncomment for final code that uses UV index and calculation
 * float UVindex = uv.readUV();
 * // the index is multiplied by 100 so to get the integer index, divide by 100!  
 * UVindex /= 100.0;  
 * Serial.print("UV: ");  Serial.println(UVindex);
 * 
 * // UV is the UV index multiplied by "uv index unit" (25 mW/m^2) and divide by 1000, to get W/m^2. multiply by interval reading (ms) times 1000, to get total Joules exposed to during that interval.
 * UV = UVindex * .025 * sensorinterval*1000;
 * UVsum = UVsum + UV; //sum of Joules exposed over time, compare to threshold
*/
 
  CircuitPlayground.setPixelColor(0, 200, 200, 200);
  delay(300);
  CircuitPlayground.setPixelColor(1, 200, 200, 200);
  delay(300);
  CircuitPlayground.setPixelColor(2, 200, 200, 200);
  delay(300);
  CircuitPlayground.setPixelColor(3, 200, 200, 200);
  delay(1000);
  CircuitPlayground.clearPixels();
  

if (UVsum > threshold){
  Serial.print("beginning of if loop");
  CircuitPlayground.setPixelColor(5, 200, 000, 000);
  CircuitPlayground.setPixelColor(6, 200, 000, 000);
  CircuitPlayground.setPixelColor(7, 200, 000, 000);
  delay(1000);
  CircuitPlayground.clearPixels();
  //playsong();
  UVsum=0; //reset sum for next reading
  Serial.print("outside of while loop ");
  Serial.println(cycles);
  blinklights();
}

  
  
  // Play the tone if the slide switch is turned on (to the left).
  if (CircuitPlayground.slideSwitch()) {
    // Play tone of the mapped frequency value for 100 milliseconds.
    CircuitPlayground.playTone(550, 100);
  }

  // Delay for a bit and repeat the loop.
  delay(sensorinterval); // wait a certain amount of time between readings

}

void blinklights(){
    while(cycles<30) {
    unsigned long currentMillis = millis();
    //Serial.println("inside the while loop");
    if (currentMillis - previousMillis >= interval) {
      // save the last time you blinked the LED
      previousMillis = currentMillis;
      Serial.println("inside if statement of while loop, should blink");
      Serial.print("value of cycles: ");
      Serial.println(cycles);
      // if the LED is off turn it on and vice-versa:
      if (ledState == LOW) {
        CircuitPlayground.setPixelColor(8, 200, 200, 000);
        CircuitPlayground.setPixelColor(9, 200, 000, 000);
        ledState = HIGH;
      } else {
        CircuitPlayground.setPixelColor(8, 000, 000, 000);
        CircuitPlayground.setPixelColor(9, 000, 000, 000);
        ledState = LOW;
      }
  
      // set the LED with the ledState of the variable:
      //digitalWrite(ledPin, ledState);
      cycles=cycles+1;
    }
   }
  cycles=0;
}

/*void colorsense(){
  bool left_first = CircuitPlayground.leftButton();
  delay(20);
  bool left_second = CircuitPlayground.leftButton();
   if (left_first && !left_second) {
    CircuitPlayground.clearPixels();
    uint8_t red, green, blue;
    CircuitPlayground.senseColor(red, green, blue);
    Serial.print("Color: red=");
    Serial.print(red, DEC);
    Serial.print(" green=");
    Serial.print(green, DEC);
    Serial.print(" blue=");
    Serial.println(blue, DEC);
    // Finally set all the pixels to the detected color.
    for (int i=0; i<10; ++i) {
      CircuitPlayground.strip.setPixelColor(i, red, green, blue);
    }
    CircuitPlayground.strip.show();
  }
}
*/

void playsong(){
  for (int thisNote = 0; thisNote < 23; thisNote++) {
    int noteDuration = 1000/noteDurations[thisNote];
    CircuitPlayground.playTone(melody[thisNote], 100, noteDuration);
    delay(noteDuration * 4 / 3); // Wait while the tone plays in the background, plus another 33% delay between notes.
  }
}

//divide the threshold in 10 parts, light up # of LEDS based on UVsum compared to threshold
void showmethelight(){
  if (UVsum < (threshold/10)){
    CircuitPlayground.setPixelColor(0, 200, 200, 200);
  }
  else if (UVsum < (2*threshold/10)){
    CircuitPlayground.setPixelColor(0, 200, 200, 200);
    CircuitPlayground.setPixelColor(1, 200, 200, 200);
  }
  else if (UVsum < (2*threshold/10)){
    CircuitPlayground.setPixelColor(0, 200, 200, 200);
    CircuitPlayground.setPixelColor(1, 200, 200, 200);
    CircuitPlayground.setPixelColor(2, 200, 200, 200);
  }
  else if (UVsum < (3*threshold/10)){
    CircuitPlayground.setPixelColor(0, 200, 200, 200);
    CircuitPlayground.setPixelColor(1, 200, 200, 200);
    CircuitPlayground.setPixelColor(2, 200, 200, 200);
    CircuitPlayground.setPixelColor(3, 200, 200, 200);
  }
  else if (UVsum < (4*threshold/10)){
    CircuitPlayground.setPixelColor(0, 200, 200, 200);
    CircuitPlayground.setPixelColor(1, 200, 200, 200);
    CircuitPlayground.setPixelColor(2, 200, 200, 200);
    CircuitPlayground.setPixelColor(3, 200, 200, 200);
    CircuitPlayground.setPixelColor(4, 200, 200, 200);
  }
  else if (UVsum < (5*threshold/10)){
    CircuitPlayground.setPixelColor(0, 200, 200, 200);
    CircuitPlayground.setPixelColor(1, 200, 200, 200);
    CircuitPlayground.setPixelColor(2, 200, 200, 200);
    CircuitPlayground.setPixelColor(3, 200, 200, 200);
    CircuitPlayground.setPixelColor(4, 200, 200, 200);
    CircuitPlayground.setPixelColor(5, 200, 200, 200);
  }  
  else if (UVsum < (6*threshold/10)){
    CircuitPlayground.setPixelColor(0, 200, 200, 200);
    CircuitPlayground.setPixelColor(1, 200, 200, 200);
    CircuitPlayground.setPixelColor(2, 200, 200, 200);
    CircuitPlayground.setPixelColor(3, 200, 200, 200);
    CircuitPlayground.setPixelColor(4, 200, 200, 200);
    CircuitPlayground.setPixelColor(5, 200, 200, 200);
    CircuitPlayground.setPixelColor(6, 200, 200, 200);
  }    
  else if (UVsum < (7*threshold/10)){
    CircuitPlayground.setPixelColor(0, 200, 200, 200);
    CircuitPlayground.setPixelColor(1, 200, 200, 200);
    CircuitPlayground.setPixelColor(2, 200, 200, 200);
    CircuitPlayground.setPixelColor(3, 200, 200, 200);
    CircuitPlayground.setPixelColor(4, 200, 200, 200);
    CircuitPlayground.setPixelColor(5, 200, 200, 200);
    CircuitPlayground.setPixelColor(6, 200, 200, 200);
    CircuitPlayground.setPixelColor(7, 200, 200, 200);
  }    
  else if (UVsum < (8*threshold/10)){
    CircuitPlayground.setPixelColor(0, 200, 200, 200);
    CircuitPlayground.setPixelColor(1, 200, 200, 200);
    CircuitPlayground.setPixelColor(2, 200, 200, 200);
    CircuitPlayground.setPixelColor(3, 200, 200, 200);
    CircuitPlayground.setPixelColor(4, 200, 200, 200);
    CircuitPlayground.setPixelColor(5, 200, 200, 200);
    CircuitPlayground.setPixelColor(6, 200, 200, 200);
    CircuitPlayground.setPixelColor(7, 200, 200, 200);
    CircuitPlayground.setPixelColor(8, 200, 200, 200);
  } 
  else if (UVsum < (7*threshold/10)){
    CircuitPlayground.setPixelColor(0, 200, 200, 200);
    CircuitPlayground.setPixelColor(1, 200, 200, 200);
    CircuitPlayground.setPixelColor(2, 200, 200, 200);
    CircuitPlayground.setPixelColor(3, 200, 200, 200);
    CircuitPlayground.setPixelColor(4, 200, 200, 200);
    CircuitPlayground.setPixelColor(5, 200, 200, 200);
    CircuitPlayground.setPixelColor(6, 200, 200, 200);
    CircuitPlayground.setPixelColor(7, 200, 200, 200);
    CircuitPlayground.setPixelColor(8, 200, 200, 200);
    CircuitPlayground.setPixelColor(9, 200, 200, 200);
  }       
}


/*  
  CircuitPlayground.playTone(349,200);
  CircuitPlayground.playTone(294,200);
  CircuitPlayground.playTone(262,200);
*/




/*
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
} //end of play alarm function
*/


/*
 *       currentMillis = millis(); // refresh current time
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
 * /
 */
