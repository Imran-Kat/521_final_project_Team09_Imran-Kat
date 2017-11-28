/* The purpose of our device is to help monitor a person's UV exposure to prevent sunburn.
 * Our device works by prompting the user to imput his/her skincolor; it then begins taking a UV reading from the sensor every 1 minute.
 * A count of total UV exposure is kept while the device is running.
 * By pressing the right button a user can view his/her current total exposure relative to their skin color threshold via the number of LEDs that light up.
 * Alarm ("You are my sunshine" song) will sound and a red LED warning will go off when total exposure reaches the max threshold for the person (based on his/her skintone input).
 *
 * Sections of  code based on Circuit Playground Analog Sensor Demo and blink without delay code from https://www.arduino.cc/en/Tutorial/BlinkWithoutDelay
 * 
 * BUTTON FUNCTIONS:
 * AT FIRST RUN: Left Button = light to dark skin color selection (increment of 3 lights)        Right Button = ENTER the selection
 * AFTER COLOR SELECTED:                                                                         Right button = show me the exposure level on LEDs
 * RESET BUTTON=ON/OFF (double click necessary)
 * 
 * FUTURE WORKS:
 * Track sunscreen application and evaluate exposure based on that parameter as well. 
 * Utilize accelerometer in order to beep the person sooner if s/he was exercising and sweating.
 * Save user's skin tone. Currently code requires skin tone input after every reset.
 * Add date and time capability, to allow automatic reset of threshold each night and track UV exposure over multiple days.
 * Set up a system for automatic skin tone recognition: http://irfankosesoy.blogspot.com/2016/10/skin-color-segmentation-matlab.html
 */

 
////////////////////////////////////////////////////////////////////////////////////////////INITIATING VARIABLES//////////////////////////////////////////////////////////////////////////////////////////


#include <Adafruit_CircuitPlayground.h>
#include <Wire.h>
#include <SPI.h> // from analog sensor demo code
#include "Adafruit_SI1145.h" // UV sensor library
#include "Adafruit_SleepyDog.h"

Adafruit_SI1145 uv = Adafruit_SI1145();

//Constants for alarm song
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


// Alarm lights blinking function
unsigned long previousMillis = 0;         // will store last time LED was updated
//unsigned long currentMillis = 0;
int ledState = LOW;                       // ledState used to set the LED
int cycles = 0;                           // used in alarm function to count number of times leds blink
const long interval = 200;                // interval at which to blink (milliseconds)
// end of alarm lights blinking function variables


// Analog light sensor on Playground, delete in final code
#define ANALOG_INPUT  A5        // Analog input A5  = light sensor

uint16_t threshold = 1000;      // Default threshold, in units of J/m^2 (probably around 300 for light skinned people), until warning alarm sounds and lights blink
int UVinterval = 10000;         // time interval between UV readings - e.g. 120,000 = every 2 minutes
unsigned long previousUVtime = 0; // holds the last time the UVreading was taken

uint32_t elapsedtime;
uint32_t currenttime;
uint32_t reapplytime;

uint16_t UV = 0;                // might need to be float, converted value of UV index into J/m^2 (how much energy has hit your skin over the time of the interval)
uint16_t UVsum = 0;             // holds the sum of all the UV exposure in J/m^2 for the user until you've reached the threshold
uint16_t blinkcounter = 0;

uint8_t SkinColor;

bool leftButtonPressed;         // left button to run color sense and get skin color
bool rightButtonPressed;        // right button to light up amount of UV on the skin so far relative to threshold - shomethelight()


////////////////////////////////////////////////////////////////////////////////////////////SET-UP////////////////////////////////////////////////////////////////////////////////////////////////////////


void setup() {
  Serial.begin(9600); // Setup serial port.
  
  CircuitPlayground.begin(); // Setup Circuit Playground library.
  Serial.println("Circuit Playground UV/photodiode sensor!");
  
  SkinColor = 1;
  CircuitPlayground.clearPixels();
  CircuitPlayground.setPixelColor(0, 0xFFFFFF);
  chooseSkinColor(); // run function to let user set their skin tone level (1, 2 or 3) before acquiring
  threshold = SkinColor * 300; // change threshold based on selected skin color (1, 2 or 3, equivalent to 300, 600 and 900 J/m^2)

}


////////////////////////////////////////////////////////////////////////////////////////////MAIN LOOP//////////////////////////////////////////////////////////////////////////////////////////////////////


void loop() {
 
  leftButtonPressed = CircuitPlayground.leftButton();
  rightButtonPressed = CircuitPlayground.rightButton();

  if(leftButtonPressed){
    Serial.println("**********************************************************Left button!");
    //CircuitPlayground.playTone(262,500);
    colorsense();
  }

  if(rightButtonPressed){
    Serial.println("**********************************************************Right button!");
    CircuitPlayground.playTone(292,100);
    showmethelight();
  }
  
  //Temp values, not using UV reading from sensor
  uint16_t value = analogRead(ANALOG_INPUT);
  //if the time interval has elapsed, save the UV measurement
  if (millis() - previousUVtime >= UVinterval) {
    UVsum = UVsum + value;
    previousUVtime = millis();
  }
    
  
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

/* uncomment for final code that uses UV index and calculation
 * float UVindex = uv.readUV();
 * // the index is multiplied by 100 so to get the integer index, divide by 100!  
 * UVindex /= 100.0;  
 * Serial.print("UV: ");  Serial.println(UVindex);
 * 
 * // UV = the UV index multiplied by "uv index unit" (25 mW/m^2) and divided by 1000, to get W/m^2. multiply by interval reading (ms) times 1000, to get total Joules exposed to during that interval.
 * UV = UVindex * .025 * sensorinterval*1000;
 * //if the time interval has elapsed, save the UV measurement
  if (millis() - previousUVtime >= UVinterval) {
    UVsum = UVsum + UV; //update the sum of Joules exposed over time, will compare to threshold
    previousUVtime = millis(); // update last time UV value was recorded
  }
*/
  for (int i=0; i<5; ++i){
      CircuitPlayground.setPixelColor(i, 220, 0, 220);
      delay(200);
  }
  CircuitPlayground.clearPixels();

  //Loop for when person has surpassed the UV threshold
    if (UVsum > threshold){
    Serial.print("beginning of if loop");
    CircuitPlayground.clearPixels();
    //playsong();
    UVsum=0; //reset sum for next reading
    Serial.print("outside of while loop "); //delete when final code working
    Serial.println(cycles); // delete when final code is working
    blinklights();
  }
  
} //end of void loop


//////////////////////////////////////////////////////////////////////////////////////////////CHOOSE SKIN COLOR FUNCTION//////////////////////////////////////////////////////////////////////////////////////


void chooseSkinColor() {
  int red = 100;
  int green = 100;
  int blue = 100;
  
  while (!CircuitPlayground.rightButton()) {
    if (CircuitPlayground.leftButton()) {
      SkinColor = SkinColor + 1;
      Serial.println("I pressed the left button");
      Serial.println(SkinColor);
      if (SkinColor > 3) SkinColor = 1;
      Serial.println(SkinColor, "after limit of 3");
      CircuitPlayground.clearPixels();
      /*
      // Define LED colors based on selected skin tone
     if (SkinColor = 1) {
        red = 100;
        green = 100;
        blue = 100;
      }
     
     else if (SkinColor = 2) {
        red = 166;
        green = 191;
        blue = 0;
      }
      
     else if (SkinColor = 3) {
        red = 124;
        green = 100;
        blue = 102;
      }
      */
      for (int p=0; p<(SkinColor*3); p++) {
        CircuitPlayground.setPixelColor(p, red, green, blue);
      }    
      delay(250); 
    }
  }
}


//////////////////////////////////////////////////////////////////////////////////////////////BLINKING LIGHTS FUNCTION///////////////////////////////////////////////////////////////////////////////////////


void blinklights(){
    while(cycles<20) {
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


///////////////////////////////////////////////////////////////////////////////////////////////COLORSENSE FUNCTION////////////////////////////////////////////////////////////////////////////////////////////


void colorsense(){
//  bool left_first = CircuitPlayground.leftButton();
//  bool left_second = CircuitPlayground.leftButton();
//   if (left_first && !left_second) {
    CircuitPlayground.clearPixels();
    uint8_t red, green, blue;
    CircuitPlayground.playTone(262,500);
    delay(200);
    CircuitPlayground.playTone(262,500);
    delay(200);
    CircuitPlayground.playTone(242,1000);
    delay(200);
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
//  }
}


////////////////////////////////////////////////////////////////////////////////////////////////SONG FUNCTION (AT THRESHOLD)///////////////////////////////////////////////////////////////////////////////////


//Song ("You are my sunshine") plays once threshold is reached
void playsong(){
  for (int thisNote = 0; thisNote < 23; thisNote++) {
    int noteDuration = 1000/noteDurations[thisNote];
    CircuitPlayground.playTone(melody[thisNote], 100, noteDuration);
    delay(noteDuration * 4 / 3); // Wait while the tone plays in the background, plus another 33% delay between notes.
  }
}


/////////////////////////////////////////////////////////////////////////////////////LEVEL OF EXPOSURE LED FUNCTION (UPON USER REQUEST)/////////////////////////////////////////////////////////////////////////


//divide the threshold in 10 parts, light up # of LEDS based on UVsum compared to threshold
void showmethelight(){
  if (UVsum < (threshold/10)){
    for (int i=0; i<1; ++i){
      CircuitPlayground.setPixelColor(i, 220, 200, 220);
      delay(200);
    }
  }
  else if (UVsum < (2*threshold/10)){
    for (int i=0; i<2; ++i){
      CircuitPlayground.setPixelColor(i, 220, 200, 220);
      delay(200);
    }
  }
  else if (UVsum < (3*threshold/10)){
    for (int i=0; i<3; ++i){
      CircuitPlayground.setPixelColor(i, 220, 200, 220);
      delay(200);
    }
  }
  else if (UVsum < (4*threshold/10)){
    for (int i=0; i<4; ++i){
      CircuitPlayground.setPixelColor(i, 220, 200, 220);
      delay(200);
    }
  }
  else if (UVsum < (5*threshold/10)){
    for (int i=0; i<5; ++i){
      CircuitPlayground.setPixelColor(i, 220, 200, 220);
      delay(200);
    }
  }
  else if (UVsum < (6*threshold/10)){
    for (int i=0; i<6; ++i){
      CircuitPlayground.setPixelColor(i, 220, 200, 220);
      delay(200);
    }
  }
  else if (UVsum < (7*threshold/10)){
    for (int i=0; i<7; ++i){
      CircuitPlayground.setPixelColor(i, 220, 200, 220);
      delay(200);
    }
  }
  else if (UVsum < (8*threshold/10)){
    for (int i=0; i<8; ++i){
      CircuitPlayground.setPixelColor(i, 220, 200, 220);
      delay(200);
    }
  }
  else if (UVsum < (9*threshold/10)){
    for (int i=0; i<9; ++i){
      CircuitPlayground.setPixelColor(i, 220, 200, 220);
      delay(200);
    }
  }
  else if (UVsum < (10*threshold/10)){
    for (int i=0; i<10; ++i){
      CircuitPlayground.setPixelColor(i, 220, 200, 220);
      delay(200);
    }
  }
  delay(1000);
  CircuitPlayground.clearPixels();
}


///////////////////////////////////////////////////////////////////////////////////////////////////////END OF CODE///////////////////////////////////////////////////////////////////////////////////////////


