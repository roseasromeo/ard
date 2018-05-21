int vpin1 = 6;
int vpin2 = 7;

bool openValves = false;

void setup() {
  // put your setup code here, to run once:
Serial.begin(9600);
Serial.println("Start");
pinMode(vpin1 ,OUTPUT);
pinMode(vpin2 ,OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
 if (openValves) {
  digitalWrite(vpin1,HIGH);
digitalWrite(vpin2,HIGH);
Serial.println("Open");
delay(500);
 }
 else {
digitalWrite(vpin1,LOW);
digitalWrite(vpin2,LOW);
Serial.println("Close");
delay(500);
 }
}

void serialEvent() {
  while(Serial.available()) Serial.read();
openValves = !openValves;
}

