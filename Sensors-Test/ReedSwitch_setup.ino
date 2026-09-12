#define IR_PIN 26

void setup() {
  Serial.begin(115200);

  pinMode(IR_PIN, INPUT);
}

void loop() {

  int irState = digitalRead(IR_PIN);

  Serial.print("IR: ");
  Serial.println(irState);

  delay(300);
}