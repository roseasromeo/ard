int vpin1 = 4;
int vpin2 = 5;
void setup() {
  // put your setup code here, to run once:
Serial.begin(9600);
Serial.println("Start");
pinMode(vpin1 ,OUTPUT);
pinMode(vpin2 ,OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
digitalWrite(vpin1,LOW);
digitalWrite(vpin2,LOW);
delay(10000);
digitalWrite(vpin1,HIGH);
digitalWrite(vpin2,HIGH);
Serial.println("Open");
delay(20000);
Serial.println("Close");
}
