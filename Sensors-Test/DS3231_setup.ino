#define REED_PIN 
void setup() {
  Serial.begin(115200);

  pinMode(REED_PIN, INPUT_PULLUP);
}

void loop() {

  if (digitalRead(REED_PIN) == LOW) {
    Serial.println("BOX CLOSED");
  } else {
    Serial.println("BOX OPEN");
  }

  delay(500);
}