int vpin = 7;
void setup() {
  // put your setup code here, to run once:
Serial.begin(9600);
Serial.println("Start");
pinMode(vpin ,OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
digitalWrite(vpin,LOW);
delay(20000);
digitalWrite(vpin,HIGH);
Serial.println("t");
delay(5000);
Serial.println("Stopt");
}
