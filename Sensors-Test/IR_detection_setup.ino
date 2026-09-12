#include "HX711.h"

#define HX711_DT 32
#define HX711_SCK 33

HX711 scale;

float calibration_factor = 210000;  // CHANGE THIS

void setup() {

  Serial.begin(115200);

  scale.begin(HX711_DT, HX711_SCK);

  scale.set_scale(calibration_factor);

  Serial.println("Taring...");
  delay(1000);

  scale.tare();

  Serial.println("Ready!");
}

void loop() {

  if (scale.is_ready()) {

    float weight = scale.get_units(5);

    Serial.print("Weight: ");
    Serial.print(weight, 3);
    Serial.println(" kg");

  } else {

    Serial.println("HX711 NOT READY");
  }

  delay(500);
}