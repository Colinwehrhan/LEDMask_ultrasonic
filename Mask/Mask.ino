/* Helper functions for an two-dimensional XY matrix of pixels.

This special 'XY' code lets you program the RGB Shades
if it's was just a plain 16x5 matrix.  

Writing to and reading from the 'holes' in the layout is 
also allowed; holes retain their data, it's just not displayed.

You can also test to see if you're on- or off- the layout
like this
  if( XY(x,y) > LAST_VISIBLE_LED ) { ...off the layout...}

X and Y bounds checking is also included, so it is safe
to just do this without checking x or y in your code:
  leds[ XY(x,y) ] == CRGB::Red;
All out of bounds coordinates map to the first hidden pixel.

 https://macetech.github.io/FastLED-XY-Map-Generator/
      0   1   2   3   4   5   6   7   8   9  10  11   12 13  14
   +-----------------------------------------------------------
 0 | 14  13  12  11  10   9   8   7   6   5   4   3   2   1   0
 1 | 15  16  17  18  19  20  21  22  23  24  25  26  27  28  29
 2 | 44  43  42  41  40  39  38  37  36  35  34  33  32  31  30
 3 | 45  46                      47                      48  49
 4 | 58  57  56              55  54  53              52  51  50
 5 | 59  60  61  62          63  64  65          66  67  68  69
 6 |     80  79  78      77  76  75  74  73      72  71  70  
 7 |                     81  82  83  84  85          
 8 |                 92  91  90  89  88  87  86        
 9 |                     93  94  95  96  97          
10 |                     102 101 100 99  98    
*/

#include <FastLED.h>
#include <EEPROM.h>
#include <JC_Button.h>

#define LED_PIN           5           // Output pin for LEDs [5]
#define COLOR_ORDER       GRB         // Color order of LED string [GRB]
#define CHIPSET           WS2812B     // LED string type [WS2182B]
#define BRIGHTNESS        255         // Overall brightness [50]
#define LAST_VISIBLE_LED  25         // Last LED that's visible [102]
#define MAX_MILLIAMPS     5000        // Max current in mA to draw from supply [500]
#define SAMPLE_WINDOW     100         // How many ms to sample audio for [100]
#define BTN_PIN           3           // Pin for button [3]
#define DEBOUNCE_MS       20          // Number of ms to debounce the button [20]
#define LONG_PRESS        500         // Number of ms to hold the button to count as long press [500]
#define PATTERN_TIME      10          // Seconds to show each pattern on autoChange [10]
#define kMatrixWidth      5          // Matrix width [15]
#define kMatrixHeight     8          // Matrix height [11]
#define NUM_LEDS (kMatrixWidth * kMatrixHeight)                                       // Total number of Leds
#define MAX_DIMENSION ((kMatrixWidth>kMatrixHeight) ? kMatrixWidth : kMatrixHeight)   // Largest dimension of matrix

#define ECHO_PIN          9           // Echo pin for ultrasonic sensor
#define TRIG_PIN          10          // Trig pin for ulatrasonic sensor

CRGB leds[ NUM_LEDS ];

// Used to check RAM availability. Usage: Serial.println(freeRam());
int freeRam () {
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

// Button stuff
uint8_t buttonPushCounter = 0;
uint8_t state = 0;
bool autoChangeVisuals = false;
Button modeBtn(BTN_PIN, DEBOUNCE_MS);

void incrementButtonPushCounter() {
  buttonPushCounter = (buttonPushCounter + 1) %11;
  EEPROM.write(1, buttonPushCounter);
}

// Include various patterns
#include "Sound.h"
#include "Rainbow.h"
#include "Fire.h"
#include "Squares.h"
#include "Circles.h"
#include "Plasma.h"
#include "Matrix.h"
#include "CrossHatch.h"
#include "Drops.h"
#include "Noise.h"
#include "Snake.h"
#include "Proximity.h"

// Helper to map XY coordinates to irregular matrix
uint16_t XY( uint8_t x, uint8_t y)
{
  // any out of bounds address maps to the first hidden pixel
  if( (x >= kMatrixWidth) || (y >= kMatrixHeight) ) {
    return (LAST_VISIBLE_LED + 1);
  }

const uint8_t XYTable[] = {
     0,  29,  30,  39,  17,
     1,  28,  31,  38,  18,
     2,  27,  32,  37,  19,
     3,  26,  33,  36,  20,
     4,  25,  34,  35,  21,
     5,  10,  11,  16,  22,
     6,   9,  12,  15,  23,
     7,   8,  13,  14,  24
  };
  
  uint8_t i = (y * kMatrixWidth) + x;
  uint8_t j = XYTable[i];
  return j;
}

void setup() {
  FastLED.addLeds < CHIPSET, LED_PIN, COLOR_ORDER > (leds, NUM_LEDS).setCorrection(TypicalSMD5050);
  //FastLED.setMaxPowerInVoltsAndMilliamps(5, MAX_MILLIAMPS);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.clear(true);

  modeBtn.begin();
  // Upload this sketch once, then uncomment the line below and upload again.
  // This ensures the EEPROM value is set correctly.
  // buttonPushCounter = (int)EEPROM.read(1);    // load previous setting

  pinMode(TRIG_PIN, OUTPUT); // Sets the trigPin as an Output
  pinMode(ECHO_PIN, INPUT); // Sets the echoPin as an Input
  
  Serial.begin(57600);
  Serial.print(F("Starting pattern "));
  Serial.println(buttonPushCounter);
}

bool checkButton() {  
  modeBtn.read(); 

  switch (state) {
    case 0:                
      if (modeBtn.wasReleased()) {
        incrementButtonPushCounter();
        Serial.print(F("Short press, pattern "));
        Serial.println(buttonPushCounter);
        autoChangeVisuals = false;
        return true;
      }
      else if (modeBtn.pressedFor(LONG_PRESS)) {
        state = 1;
        return true;
      }
      break;

    case 1:
      if (modeBtn.wasReleased()) {
        state = 0;
        Serial.print(F("Long press, auto, pattern "));
        Serial.println(buttonPushCounter);
        autoChangeVisuals = true;
        return true;
      }
      break;
  }

  if(autoChangeVisuals){
    EVERY_N_SECONDS(PATTERN_TIME) {
      incrementButtonPushCounter();
      Serial.print("Auto, pattern ");
      Serial.println(buttonPushCounter); 
      return true;
    }
  }  

  return false;
}

// Functions to run patterns. Done this way so each class stays in scope only while
// it is active, freeing up RAM once it is changed.

void runSound(){
  bool isRunning = true;
  Proximity proximity = Proximity();
  while(isRunning) isRunning = proximity.runPattern();
}

void runRainbow(){
  bool isRunning = true;
  Rainbow rainbow = Rainbow();
  while(isRunning) isRunning = rainbow.runPattern();
}

void runFire(){
  bool isRunning = true;
  Fire fire = Fire();
  while(isRunning) isRunning = fire.runPattern();
}

void runSquares(){
  bool isRunning = true;
  Squares squares = Squares();
  while(isRunning) isRunning = squares.runPattern();
}

void runCircles(){
  bool isRunning = true;
  Circles circles = Circles();
  while(isRunning) isRunning = circles.runPattern();
}

void runPlasma(){
  bool isRunning = true;
  Plasma plasma = Plasma();
  while(isRunning) isRunning = plasma.runPattern();
}

void runMatrix(){
  bool isRunning = true;
  Matrix matrix = Matrix();
  while(isRunning) isRunning = matrix.runPattern();
}

void runCrossHatch(){
  bool isRunning = true;
  CrossHatch crossHatch = CrossHatch();
  while(isRunning) isRunning = crossHatch.runPattern();
}

void runDrops(){
  bool isRunning = true;
  Drops drops = Drops();
  while(isRunning) isRunning = drops.runPattern();
}

void runNoise(){
  bool isRunning = true;
  Noise noise = Noise();
  while(isRunning) {
    isRunning = noise.runPattern();
  }
}

void runSnake(){
  bool isRunning = true;
  Snake snake = Snake();
  while(isRunning) {
    isRunning = snake.runPattern();
  }
}

// Run selected pattern
void loop() {
  switch (buttonPushCounter) {
    case 0:
      runSound();
      break;
    case 1:
      runRainbow();
      break;
    case 2:
      runFire();
      break;
    case 3:
      runSquares();
      break;
    case 4:
      runCircles();
      break;
    case 5:
      runPlasma();
      break;
    case 6:
      runMatrix();
      break;
    case 7:
      runCrossHatch();
      break;
    case 8:
      runDrops();
      break;
    case 9:
      runNoise();
      break;
    case 10:
      runSnake();
      break;
  }
}
