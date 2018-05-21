int const valve1Pin = 2;
int const switchPin = 4;
int const ledPin = 13;

int switchState = LOW;

void setup() {
  // put your setup code here, to run once:
  pinMode(valve1Pin, OUTPUT);
  pinMode(switchPin, INPUT);
  pinMode(ledPin, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  //switchState = digitalRead(switchPin);
  //if (switchState == HIGH) {
    delay(2000);
    digitalWrite(ledPin, HIGH);
    digitalWrite(valve1Pin, HIGH);
    Serial.println("on");
    delay(5000);
  //} else {
    digitalWrite(ledPin, LOW);
    digitalWrite(valve1Pin, LOW);
    Serial.println("off");
    delay(200);
  //}
}

