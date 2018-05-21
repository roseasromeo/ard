int vpin1 = 6;
int vpin2 = 7;
bool openValve = false;

void setup() {
  // put your setup code here, to run once:
Serial.begin(9600);
Serial.println("Start");
pinMode(vpin1 ,OUTPUT);
pinMode(vpin2 ,OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
 if (openValve) {
  digitalWrite(vpin1,HIGH);
  digitalWrite(vpin2,HIGH);
  Serial.println("Open");
 }
 else {
  digitalWrite(vpin1,LOW);
  digitalWrite(vpin2,LOW);
  Serial.println("Close");
 }

delay(1000);

}

void serialEvent() {
  while (Serial.available()) Serial.read();
  openValve = !openValve;
}

