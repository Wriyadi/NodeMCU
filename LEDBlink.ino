void setup() {
  pinMode(16, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(0, OUTPUT);
  pinMode(2, OUTPUT);
  pinMode(14, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(13, OUTPUT);
  pinMode(15, OUTPUT);
}

void loop() {
  //LOW to MAX
  digitalWrite(16, HIGH);
  delay(100);
  digitalWrite(5, HIGH);
  delay(100);
  digitalWrite(4, HIGH);
  delay(100);
  digitalWrite(0, HIGH);
  delay(100);
  digitalWrite(2, HIGH);
  delay(100);
  digitalWrite(14, HIGH);
  delay(100);
  digitalWrite(12, HIGH);
  delay(100);
  digitalWrite(13, HIGH);
  delay(100);
  digitalWrite(15, HIGH);
  delay(500);

  digitalWrite(15, LOW);
  delay(100);
  digitalWrite(13, LOW);
  delay(100);
  digitalWrite(12, LOW);
  delay(100);
  digitalWrite(14, LOW);
  delay(100);
  digitalWrite(2, LOW);
  delay(100);
  digitalWrite(0, LOW);
  delay(100);
  digitalWrite(4, LOW);
  delay(100);
  digitalWrite(5, LOW);
  delay(100);
  digitalWrite(16, LOW);
  delay(500);

  //STROBO
  digitalWrite(16, HIGH);
  digitalWrite(5, HIGH);
  digitalWrite(4, HIGH);
  digitalWrite(0, HIGH);
  delay(100);
  digitalWrite(16, LOW);
  digitalWrite(5, LOW);
  digitalWrite(4, LOW);
  digitalWrite(0, LOW);
  delay(100);
  digitalWrite(16, HIGH);
  digitalWrite(5, HIGH);
  digitalWrite(4, HIGH);
  digitalWrite(0, HIGH);
  delay(100);
  digitalWrite(16, LOW);
  digitalWrite(5, LOW);
  digitalWrite(4, LOW);
  digitalWrite(0, LOW);
  delay(100);

  digitalWrite(14, HIGH);
  digitalWrite(12, HIGH);
  digitalWrite(13, HIGH);
  digitalWrite(15, HIGH);
  delay(100);
  digitalWrite(14, LOW);
  digitalWrite(12, LOW);
  digitalWrite(13, LOW);
  digitalWrite(15, LOW);
  delay(100);
  digitalWrite(14, HIGH);
  digitalWrite(12, HIGH);
  digitalWrite(13, HIGH);
  digitalWrite(15, HIGH);
  delay(100);
  digitalWrite(14, LOW);
  digitalWrite(12, LOW);
  digitalWrite(13, LOW);
  digitalWrite(15, LOW);
  delay(500);

  //LED Chaser Kanan
  digitalWrite(16, HIGH);
  delay(100);
  digitalWrite(16, LOW);
  digitalWrite(5, HIGH);
  delay(100);
  digitalWrite(5, LOW);
  digitalWrite(4, HIGH);
  delay(100);
  digitalWrite(4, LOW);
  digitalWrite(0, HIGH);
  delay(100);
  digitalWrite(0, LOW);
  digitalWrite(2, HIGH);
  delay(100);
  digitalWrite(2, LOW);
  digitalWrite(14, HIGH);
  delay(100);
  digitalWrite(14, LOW);
  digitalWrite(12, HIGH);
  delay(100);
  digitalWrite(12, LOW);
  digitalWrite(13, HIGH);
  delay(100);
  digitalWrite(13, LOW);
  digitalWrite(15, HIGH);
  delay(100);
  digitalWrite(15, LOW);
  delay(500);

  //LED Chaser KIRI
  digitalWrite(15, HIGH);
  delay(100);
  digitalWrite(15, LOW);
  digitalWrite(13, HIGH);
  delay(100);
  digitalWrite(13, LOW);
  digitalWrite(12, HIGH);
  delay(100);
  digitalWrite(12, LOW);
  digitalWrite(14, HIGH);
  delay(100);
  digitalWrite(14, LOW);
  digitalWrite(2, HIGH);
  delay(100);
  digitalWrite(2, LOW);
  digitalWrite(0, HIGH);
  delay(100);
  digitalWrite(0, LOW);
  digitalWrite(4, HIGH);
  delay(100);
  digitalWrite(4, LOW);
  digitalWrite(5, HIGH);
  delay(100);
  digitalWrite(5, LOW);
  digitalWrite(16, HIGH);
  delay(100);
  digitalWrite(16, LOW);
  delay(500);

  //Luar Ke Dalam
  digitalWrite(16, HIGH);
  digitalWrite(15, HIGH);
  delay(100);
  digitalWrite(16, LOW);
  digitalWrite(15, LOW);
  delay(100);
  digitalWrite(5, HIGH);
  digitalWrite(13, HIGH);
  delay(100);
  digitalWrite(5, LOW);
  digitalWrite(13, LOW);
  delay(100);
  digitalWrite(4, HIGH);
  digitalWrite(12, HIGH);
  delay(100);
  digitalWrite(4, LOW);
  digitalWrite(12, LOW);
  delay(100);
  digitalWrite(0, HIGH);
  digitalWrite(14, HIGH);
  delay(100);
  digitalWrite(0, LOW);
  digitalWrite(14, LOW);
  delay(100);
  digitalWrite(2, HIGH);
  delay(100);
  digitalWrite(2, LOW);
  delay(200);

  //Dalam Ke Luar
  digitalWrite(0, HIGH);
  digitalWrite(14, HIGH);
  delay(100);
  digitalWrite(0, LOW);
  digitalWrite(14, LOW);
  delay(100);
  digitalWrite(4, HIGH);
  digitalWrite(12, HIGH);
  delay(100);
  digitalWrite(4, LOW);
  digitalWrite(12, LOW);
  delay(100);
  digitalWrite(5, HIGH);
  digitalWrite(13, HIGH);
  delay(100);
  digitalWrite(5, LOW);
  digitalWrite(13, LOW);
  delay(100);
  digitalWrite(16, HIGH);
  digitalWrite(15, HIGH);
  delay(100);
  digitalWrite(16, LOW);
  digitalWrite(15, LOW);
  delay(500);
}
