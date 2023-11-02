// Falling green 'code' like The Matrix.
// Adapted from https://gist.github.com/Jerware/b82ad4768f9935c8acfccc98c9211111

#include "Arduino.h"
#include "UltrasonicSensor.h"

class Matrix {
  public:
    Matrix(){};
    bool runPattern();
  private:
    void readDistance();
    long previousTime = 0;
    int distance = 0;
    int spawnRate = 0;
    double rateLimit =0;
};

bool Matrix::runPattern() {
  // initilize read distance class
  ultraSonicSensor sensor; // Create an instance of ReadDistance

  if(checkButton()) return false;
  if(millis() - previousTime >= 75) {
    // Move bright spots downward
    for (int row = kMatrixHeight - 1; row >= 0; row--) {
      for (int col = 0; col < kMatrixWidth; col++) {
        if (leds[XY(col, row)] == CRGB(175,255,175)) {
          leds[XY(col, row)] = CRGB(27,200,39); // create trail
          if (row < kMatrixHeight - 1) leds[XY(col, row + 1)] = CRGB(175,255,175);
        }
      }
    }
    
    // Fade all leds
    for(int i = 0; i < NUM_LEDS; i++) {
      if (leds[i].g != 255) leds[i].nscale8(192); // only fade trail
    }
    // get the distance from ultrasonic
    distance = sensor.readDistance();

    // half it for effect
    //rateLimit = distance;

    // square root it to increase freq
    spawnRate = sqrt(distance);
    //spawnRate = pow(rateLimit, 1.0 / 4.0 );



    // Prints the distance on the Serial Monitor
    Serial.print("spawnRate: ");
    Serial.println(spawnRate);


    // Spawn new falling spots
    if (random8(spawnRate) == 0) // lower number == more frequent spawns
    {
      int8_t spawnX = random8(kMatrixWidth);
      leds[XY(spawnX, 0)] = CRGB(175,255,175 );
    }

    FastLED.show();
    previousTime = millis();
  }
  return true;
}
