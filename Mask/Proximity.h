// Basic rainbow effect.

#include "Arduino.h"


class Proximity {
  public:
    Proximity(){};
    bool runPattern();
  private:
    void drawOneFrame(byte startHue8, int8_t yHueDelta8, int8_t xHueDelta8);
    ultraSonicSensor sensorProx; // Define the sensor as a member variable
    int distance;
    int brightness;
    int count = 0;
};

bool Proximity::runPattern() {
  if(checkButton()) return false;
  uint32_t ms = millis();
  count++;
  int32_t yHueDelta32 = ((int32_t) cos16(count * (27 / 1)) * (350 / kMatrixWidth));
  int32_t xHueDelta32 = ((int32_t) cos16(count * (39 / 1)) * (310 / kMatrixHeight));
              // get distance and brightness
      distance = sensorProx.readDistance();
      brightness = sensorProx.mapBrightness(distance);
  drawOneFrame(count / 65536, yHueDelta32 / 32768, xHueDelta32 / 32768);
  FastLED.show();
  return true;
}

void Proximity::drawOneFrame(byte startHue8, int8_t yHueDelta8, int8_t xHueDelta8) {
  byte lineStartHue = startHue8;

  for (byte x = 0; x < kMatrixWidth; x++) {
  for (byte y = 0; y < kMatrixHeight; y++) {
    lineStartHue += yHueDelta8;
    byte pixelHue = lineStartHue;

      pixelHue += xHueDelta8;

      leds[XY(x, y)] = CHSV(pixelHue, 255, brightness);
    }
  }
}
