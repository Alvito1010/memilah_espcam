#include "infrared.h"

void InfraredSensor() {

  int state = digitalRead(SENSOR_PIN);

  if (state == LOW) {
    delay(1000);
    ObjectDetected = true;
  }
  else {
    ObjectDetected = false;
  }

}