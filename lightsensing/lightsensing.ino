/* The purpose of our device is to help monitor a person's UV exposure to prevent sunburn.
 * Our device works by prompting the user to input his/her skincolor; it then begins taking a UV reading from the sensor every 1 minute.
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
#include "Adafruit_SI1145.h" // UV sensor library

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
int ledState = LOW;                       // ledState used to turn LEDs on and off
int cycles = 0;                           // used in alarm function to count number of times LEDs blink
const long interval = 200;                // interval at which to blink (milliseconds)
// end of alarm lights blinking function variables


uint16_t threshold;                       // Threshold, in units of J/m^2 (probably around 300 for light skinned people), until warning alarm sounds and lights blink
int UVinterval = 60000;                   // time interval between recording UV readings - e.g. 60,000 = every 1 minute
unsigned long previousUVtime = 0;         // holds the last time the UVreading was taken


uint16_t UV = 0;                          // converted value of UV index into J/m^2 (how much energy has hit your skin over the time of the interval)
uint16_t UVsum = 0;                       // holds the sum of the total UV exposure in J/m^2 for the user until you've reached the threshold
uint16_t blinkcounter = 0;

uint8_t SkinColor;                        // value between 1 and 3, light to dark skin. User defined.

bool leftButtonPressed;                   // left button to run color sense and get skin color
bool rightButtonPressed;                  // right button to light up amount of UV on the skin so far relative to threshold - shomethelight()


////////////////////////////////////////////////////////////////////////////////////////////SET-UP////////////////////////////////////////////////////////////////////////////////////////////////////////


void setup() {
  Serial.begin(9600);
  CircuitPlayground.begin(); // Setup Circuit Playground library.
  
  SkinColor = 1;
  CircuitPlayground.clearPixels();
  CircuitPlayground.setPixelColor(0, 0xFFFFFF);
  chooseSkinColor();                    // run function to let user set their skin tone level (1, 2 or 3) before acquiring
  threshold = SkinColor * 300;          // change threshold based on selected skin color (1, 2 or 3, equivalent to 300, 600 and 900 J/m^2)

}


////////////////////////////////////////////////////////////////////////////////////////////MAIN LOOP//////////////////////////////////////////////////////////////////////////////////////////////////////


void loop() {
 
  leftButtonPressed = CircuitPlayground.leftButton();
  rightButtonPressed = CircuitPlayground.rightButton();

  /* Currently not running because automatic skin color sensing functions is not accurate or reproducible
  if(leftButtonPressed){
    Serial.println("**********************************************************Left button!");
    colorsense();
  }
  */

  if(rightButtonPressed){
    Serial.println("**********************************************************Right button!");
    CircuitPlayground.playTone(292,100);
    showmethelight();                 // light up amount of UV on the skin so far relative to threshold
  }
  
  float UVindex = uv.readUV();
  UVindex /= 100.0;                   // the index is multiplied by 100 so to get the integer index, divide by 100!
 
  // UV = the UV index multiplied by "uv index unit" (25 mW/m^2) and divided by 1000, to get W/m^2. multiply by interval reading (in seconds - milliseconds divided by 1000), to get total Joules exposed to during that interval.
  UV = UVindex * .025 * (UVinterval / 1000);
  //if the time interval has elapsed, save the UV measurement
  if (millis() - previousUVtime >= UVinterval) {
    UVsum = UVsum + UV;               // update the sum of Joules exposed over time, will compare to threshold
    previousUVtime = millis();        // update last time UV value was recorded
  }


  // Light up a few LEDs to show device is working
  for (int i=0; i<5; ++i){
      CircuitPlayground.setPixelColor(i, 220, 0, 220);
      delay(200);
  }
  CircuitPlayground.clearPixels();

  //Loop for when person has surpassed the UV threshold
  if (UVsum > threshold){
    CircuitPlayground.clearPixels();
    UVsum=0;                          // reset sum for next reading
    blinklights();                    // Alarm lights
    playsong();                       // Alarm song
  }
  
} //end of void loop


//////////////////////////////////////////////////////////////////////////////////////////////CHOOSE SKIN COLOR FUNCTION//////////////////////////////////////////////////////////////////////////////////////


void chooseSkinColor() {
  int red = 100;
  int green = 100;
  int blue = 100;

  // Left button to cycle through skin tone levels (1-3, light to dark)
  // Right button to enter selection and start measuring UV
  while (!CircuitPlayground.rightButton()) {
    if (CircuitPlayground.leftButton()) {
      SkinColor = SkinColor + 1;
      if (SkinColor > 3) SkinColor = 1;
      CircuitPlayground.clearPixels();

      // Light up LEDs in increments of 3 for each level
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
  
      cycles=cycles+1;
    }
   }
  cycles=0;                           // Reset blink cycles until for next alarm
}


///////////////////////////////////////////////////////////////////////////////////////////////COLORSENSE FUNCTION////////////////////////////////////////////////////////////////////////////////////////////
/* To be used in future code once a more accurate skin color identification can be developed. Will be used to automatically adjust threshold instead of user input.

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

*/
////////////////////////////////////////////////////////////////////////////////////////////////SONG FUNCTION (AT THRESHOLD)///////////////////////////////////////////////////////////////////////////////////


// Song ("You are my sunshine") plays once threshold is reached
void playsong(){
  for (int thisNote = 0; thisNote < 23; thisNote++) {
    int noteDuration = 1000/noteDurations[thisNote];
    CircuitPlayground.playTone(melody[thisNote], 100, noteDuration);
    delay(noteDuration * 4 / 3);      // Wait while the tone plays in the background, plus another 33% delay between notes.
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


