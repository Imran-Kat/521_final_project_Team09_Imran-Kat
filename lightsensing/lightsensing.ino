/* Takes UV reading from sensor every 5 minutes.
 * Summation counter keeps tab of total UV exposure each day, so add newest reading to counter.
 * Lights will increase in number and intensity as total UV exposure (summation counter) increases.
 * Alarm will sound when total exposure reaches certain max threshold.
 * Right button will mute alarm and assume more sunscreen was applied. Will only increase UV counter summation by 5% for next 2 hours because assumes suncreen blocks 95% of UV light.
 * If you don't press button in the alarm time (30 seconds), assumes you didn't apply sunscreen.
 * Reset button will reset counter for each new day or if device is worn by a new person in the middle of the day.
 * Each day, summation counter resets if date and time works on playground.
 * Left button will be used to make skin color measurement.
 * 
 * 
 */

// Circuit Playground Analog Sensor Demo
// Shows how to read an analog sensor like temperature, light,
// sound, or even external inputs and convert the analog value
// to color and sound on the board.  Will light up NeoPixel 4 and 5
// with a color proportional to the analog value, and if the slide
// switch is turned to the left will play a music tone proportional
// to the value.
#include <Adafruit_CircuitPlayground.h>
#include <Wire.h>
#include <SPI.h>

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
uint16_t UVsum = 0;

void setup() {
  // Setup serial port.
  Serial.begin(115200);
  Serial.println("Circuit Playground Analog Sensor Demos!");
  // Setup Circuit Playground library.
  CircuitPlayground.begin();
}

void loop() {
  // Get the sensor sensor value and print it out (can use serial plotter
  // to view realtime graph!).
  uint16_t value = analogRead(ANALOG_INPUT);
  Serial.println(value, DEC);
  UVsum = UVsum + value;
  Serial.println(UVsum, DEC);

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
