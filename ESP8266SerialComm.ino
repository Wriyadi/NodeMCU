const int led1 = 16;
const int led2 = 5;
const int led3 = 4;
const int led4 = 0;
const int led5 = 2;
int inByte = 0; 

void setup() {
  // start serial port at 9600 bps:
  Serial.begin(115200);
  while (!Serial) {
    ;  // wait for serial port to connect. Needed for native USB port only
  }

  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
  pinMode(led4, OUTPUT);
  pinMode(led5, OUTPUT);
  establishContact();  // send a byte to establish contact until receiver responds
}

void loop() {
  // if we get a valid byte, read analog ins:
  if (Serial.available() > 0) {
    // get incoming byte:
    inByte = Serial.read();
    // read first analog input, divide by 4 to make the range 0-255:
    if (inByte == 1){
      digitalWrite(led1,HIGH);
      digitalWrite(led2,LOW);
      digitalWrite(led3,LOW);
      digitalWrite(led4,LOW);
      digitalWrite(led5,LOW);
    } else if (inByte == 2){
      digitalWrite(led1,HIGH);
      digitalWrite(led2,HIGH);
      digitalWrite(led3,LOW);
      digitalWrite(led4,LOW);
      digitalWrite(led5,LOW);    
    } else if (inByte == 3){
      digitalWrite(led1,HIGH);
      digitalWrite(led2,HIGH);
      digitalWrite(led3,HIGH);
      digitalWrite(led4,LOW);
      digitalWrite(led5,LOW);    
    } else if (inByte == 4){
      digitalWrite(led1,HIGH);
      digitalWrite(led2,HIGH);
      digitalWrite(led3,HIGH);
      digitalWrite(led4,HIGH);
      digitalWrite(led5,LOW);    
    } else if (inByte == 5){
      digitalWrite(led1,HIGH);
      digitalWrite(led2,HIGH);
      digitalWrite(led3,HIGH);
      digitalWrite(led4,HIGH);
      digitalWrite(led5,HIGH);
    } else{
      digitalWrite(led1,LOW);
      digitalWrite(led2,LOW);
      digitalWrite(led3,LOW);
      digitalWrite(led4,LOW);
      digitalWrite(led5,LOW);      
    }
  }
}

void establishContact() {
  while (Serial.available() <= 0) {
    delay(100);
  }
}
