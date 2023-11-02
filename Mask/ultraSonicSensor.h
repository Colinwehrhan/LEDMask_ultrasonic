#include <Arduino.h> // Include the Arduino library if not already included


class ultraSonicSensor {
  public:
    ultraSonicSensor() {};
        // Constructor (no parameters needed)
    int readDistance();
  private:
    int distance = 0;
    int duration = 0;
};


int ultraSonicSensor::readDistance() {
// Clears the TRIG_PIN
digitalWrite(TRIG_PIN, LOW);
delayMicroseconds(2);

// Sets the TRIG_PIN on HIGH state for 10 microseconds
digitalWrite(TRIG_PIN, HIGH);
delayMicroseconds(10);
digitalWrite(TRIG_PIN, LOW);

// Reads the ECHO_PIN, returns the sound wave travel time in microseconds
duration = pulseIn(ECHO_PIN, HIGH);

// Calculating the distance
distance = duration * 0.034 / 2;

// Prints the distance on the Serial Monitor
Serial.print("Distance: ");
Serial.println(distance);
return distance;
}

