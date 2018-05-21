// Setup load cell object
#include <HX711.h>
// scale(DT,SCK)
HX711 scaleA(A1, A0); //Scale A HX711 chip object
HX711 scaleB(A3, A2); //Scale B HX711 chip object
const int pressurePinA = A5; //green wire on tank A's pressure transducer
const int pressurePinB = A4; //green wire on tank B's pressure transducer
#define PSI_0 104  //analog reading at 1 atm/ 0 PSI gauge pressure
#define PSI_300 922 //predicted analog reading at 300 PSI gauge pressure
                                // needs to be confirmed when we have the pressure line available
int PSIA() { //function to determine pressure in tank A
int voltsA  = analogRead(pressurePinA);
int pressure = map(voltsA, PSI_0, PSI_300, 0, 300);
return pressure;
}
int PSIB() { //function to determine pressure in tank B
int voltsB = analogRead(pressurePinB);
int pressure = map(voltsB, PSI_0, PSI_300, 0, 300);
 return pressure;
}

// Calibration variables for scale
long offset_A = 0;
long offset_B = 0;
float scale_A = 1;
float scale_B = 1;
// Pin assignments
int valveA1Pin = 2;
int valveA2Pin = 3;
int valveB1Pin = 4; //A1 goes to B1 and A2 goes to B2
int valveB2Pin = 5; //A1 to B1 is long track, B2 to A2 is short track
int triValveAPin = 6;
int triValveBPin = 7;
// Timing
int pressureTime = 500; //Change after testing pressurization 
unsigned long runTime = 0;
unsigned long resetTime = 0;
float printTime = 0;

bool path; // true is long track, false is short track (from control on LabVIEW panel)
int ValvePins[] = {triValveAPin, triValveBPin, valveA1Pin, valveA2Pin, valveB1Pin, valveB2Pin};

bool error = false;
// Initialize serial read variable
int incomingCom = 0;

void setup() {
  // Start Serial communications
  Serial.begin(9600);
  for ( int thisPin = 0; thisPin <= 5; thisPin++) {
    pinMode(ValvePins[thisPin], OUTPUT);
  }
  //Serial.print("done");
  }
  //pinMode(cameraPin) = OUTPUT;


void loop() {
  // put your main code here, to run repeatedly:
  
 if(error == true){
    for ( int thisPin = 0; thisPin <= 5; thisPin++) {
      digitalWrite(ValvePins[thisPin], LOW);
      
  }
  Serial.println("Error");
  }
  //If nothing else is happening, return the masses and the timestamp
 else{
  runTime = millis() - resetTime;
  printTime = float(runTime/float(1000));
  Serial.print(printTime);
  Serial.print("     ");
  Serial.print(" A: "); //label output
  readCell1(10);
  Serial.print("    B: ");
  readCell2(10);
 }
}

void serialEvent() {
  //Whenever the Arduino receives a serial command, react
    incomingCom = Serial.read(); //Get the first character of the command


    //List of commands:
    // a Tare
    // b Wt1,Wt2 Calibrate scale A
    // c Output current settings
    // d scA.A,scB.B Set settings
    // g reset time
    // h Calibrate scale B
  
    if (incomingCom == 'a') {
      //labview-friendly tare
      tareCells(50);
    }
  
    
    else if (incomingCom == 'b') {
      //labview-friendly calibration--need to get two weights in same units with command
      //i.e. B Wt1,Wt2
      calibrationRead(true);
    }
    
    else if (incomingCom == 'c') {
      //Print settings without labels (labview-friendly)
      
      Serial.println("Offset: ");
      Serial.println(offset_A);
     Serial.println(offset_B);
     Serial.println("Scale 1: ");
      Serial.println(scale_A, 4);
      Serial.println("Scale 2: ");
      Serial.println(scale_B, 4);
    }
    
    else if (incomingCom == 'd') {
      //Set settings
      settings();
    }
    
    
    else if(incomingCom == 'e'){
      //Open valves
      if(path) {
        digitalWrite(triValveBPin, LOW);
        digitalWrite(triValveAPin, HIGH);
        delay (pressureTime);
        Serial.print(printTime);
        Serial.print("PUSH");
        Serial.print("Tank A Pressure: ");
       Serial.println(PSIA());
        Serial.print(printTime);
        Serial.print("PUSH");
        Serial.print("Tank B Pressure: ");
       Serial.print(PSIB());
        digitalWrite(valveB1Pin, HIGH);
        digitalWrite(valveA1Pin,HIGH);
        Serial.print(printTime);
        Serial.print("PUSH");
        Serial.println("  Long Path Open");
      }
      else if(!path) {
        digitalWrite(triValveAPin, LOW);
        digitalWrite(triValveBPin, HIGH);
        delay (pressureTime);
        Serial.print(printTime);
        Serial.print("PUSH");
        Serial.print("Tank A Pressure: ");
       Serial.println(PSIA());
        Serial.print(printTime);
        Serial.print("PUSH");
        Serial.print("Tank B Pressure: ");
      Serial.println(PSIB());
        digitalWrite(valveA2Pin, HIGH);
        digitalWrite(valveB2Pin, HIGH);
        Serial.print(printTime);
        Serial.print("PUSH");
        Serial.println("   Short Path Open");       
      }
      else {
        error = true;
      }
    }
    
    else if(incomingCom == 'f'){
      //Close valves
      if (path) {
        digitalWrite(triValveAPin, LOW);
        digitalWrite(valveA1Pin, LOW);
        digitalWrite(valveB1Pin, LOW);
        Serial.print(printTime);
        Serial.print("PUSH");
        Serial.print("Tank A Pressure: ");
       Serial.println(PSIA());
        Serial.print(printTime);
        Serial.print("PUSH");
        Serial.print("Tank B Pressure: ");
       Serial.println(PSIB());
        Serial.print(printTime);
        Serial.print("PUSH");
        Serial.println("   Long Path Closed");
      }
      else if (!path) {
        digitalWrite(triValveBPin, LOW);
        digitalWrite(valveB2Pin, LOW);
        digitalWrite(valveA2Pin, LOW);
        Serial.print(printTime);
        Serial.print("PUSH");
        Serial.print("Tank A Pressure: ");
      Serial.println(PSIA());
        Serial.print(printTime);
        Serial.print("PUSH");
        Serial.print("Tank B Pressure: ");
        Serial.println(PSIB());
        Serial.print(printTime);
        Serial.print("PUSH");
        Serial.println("  Short Path Closed");
      }      
      else {
        error = true;
      }
    }
   
    else if(incomingCom == 'g'){
     
      resetTime = millis();  
    }
    
    else if(incomingCom == 'h'){
      calibrationRead(false);
    }
    
    else if (incomingCom == 'L'){
      path = true;
    }
    else if (incomingCom == 'S'){
      path = false;
    }
    else if (incomingCom == 'E') {
      error = true;
    }
}

void readCell1(int repeats) {
  scaleA.set_gain(128);
  float reading = scaleA.read_average(repeats);
  Serial.print((reading - offset_A) / scale_A, 4);
}

void readCell2(int repeats) {
  scaleB.set_gain(128);
  float reading = scaleB.read_average(repeats);
  Serial.println((reading - offset_B) / scale_B, 4);
}

void tareCells(int repeats) {
   while (Serial.available()) Serial.read();
  
  while (!Serial.available()) {
  scaleA.set_gain(128);
  float reading = scaleA.read_average(repeats);
  offset_A = reading;
  }
  Serial.println(offset_A);
  
  while (Serial.available()) Serial.read();
  while(!Serial.available()) {
  scaleB.set_gain(128);
  float reading = scaleB.read_average(repeats);
  offset_B = reading;
  }
  Serial.println(offset_B);

  while (!Serial.available());
  Serial.print(printTime);
  Serial.print("  PUSH");
  Serial.println("Tare complete.");
}

void calibrationRead(bool scaleIdent) {
  int weight1 = 0;
  int weight2 = 0;
  int weightnum = 0;
  //calibration--need to get two weights in same units with command
  while (Serial.available()) {
    int incoming = Serial.read();
    if (incoming > 47 && incoming < 58) {
      if (weightnum == 0) {
        weight1 = (weight1 * 10) + (incoming - 48);
      }
      else if (weightnum == 1) {
        weight2 = (weight2 * 10) + (incoming - 48);
      }
      else {
        break;
      }
    }
    else if (incoming == ',') {
      weightnum++;
    }
  }
  if (weight1 && weight2) {   
    calibration(50, weight1, weight2, scaleIdent);
    Serial.print(printTime);
    Serial.print("PUSH");
    Serial.print("   Calibration complete.");
    if (scaleIdent){
      Serial.println("Scale A");
    }
    else{
      Serial.println("Scale B");
    }
  }
  
    else {
      Serial.print(printTime);
    Serial.println("   Calibration failed due to invalid weight entry. Format: b Wt1,Wt2");
    error = true;
    }
  }


void calibration(int repeats, int weight1, int weight2, bool scaleIdent) {
  int readingA1 = 0;
  int readingA2 = 0;
  int readingB1 = 0;
  int readingB2 = 0;
  
  while (Serial.available()) Serial.read();
  
  while (!Serial.available()); //wait for any command to be sent
  if (scaleIdent) {
    scaleA.set_gain(128);
    long readingA1 = scaleA.read_average(repeats);
    Serial.println(readingA1);
  }
  else {
    scaleB.set_gain(128);
    long readingB1 = scaleB.read_average(repeats);
    Serial.println(readingB1);
  }

  while (Serial.available()) Serial.read();
 
 while (!Serial.available()); //wait for any command to be sent
  if (scaleIdent) {
    scaleA.set_gain(128);
    long readingA2 = scaleA.read_average(repeats);
    Serial.println(readingA2);
    //scale_A = float((readingA2 - readingA1) / (weight2 - weight1));
  }
  else {
    scaleB.set_gain(128);
    long readingB2 = scaleB.read_average(repeats);
    Serial.println(readingB2);
    //scale_B = float((readingB2 - readingB1) / (weight2 - weight1));
  }
}

void settings() {
  //Initialize storage variables for inputs
  float scA1 = 0; //before decimal point
  float scA2 = 0; //after decimal point
  float scB1 = 0; //before decimal point
  float scB2 = 0; //after decimal point

  //Initialize variables needed for calculations/reading
  int settingnum = 0;
  int counterA = 1;
  int counterB = 1;
  bool switchA = false;
  bool switchB = false;

  //settings--need to get 2 numbers with command
  //i.e. scA.A,scB.B
  while (Serial.available()) {
    int incoming = Serial.read();
    if (incoming > 47 && incoming < 58) {
       if (settingnum == 0) {
        if (switchA) {
          scA2 = (scA2 * 10) + (incoming - 48);
          counterA = counterA * 10; //decimal part
        }
        else {
          scA1 = (scA1 * 10) + (incoming - 48); //whole number part
        }
      }
      else if (settingnum == 1) {
        if (switchB) {
          scB2 = (scB2 * 10) + (incoming - 48); //decimal part
          counterB = counterB * 10;
        }
        else {
          scB1 = (scB1 * 10) + (incoming - 48); //whole number part
        }
      }
      else {
        break; //discard extra characters, handle the case where last comma is omitted (standard practice)
      }
    }
    else if (incoming == '-'){
      continue;
    }
    else if (incoming == ',') {
      settingnum++;
    }
    else if (incoming == '.') {
      
        if (settingnum == 1) {
          switchB = true;
        }
        if (settingnum == 0) {
          switchA = true;
        }
      }
    
      else {
       error = true;
     }
    }
   
  if (scA1 && scB1 && !error) { 
    scale_A = scA1 + scA2 / counterA;
    scale_B = scB1 + scB2 / counterB;
      Serial.print(printTime);
      Serial.print("PUSH");
      Serial.println("   Settings complete.");
  }

    else {
      Serial.print(printTime);
      Serial.println("   Settings failed due to invalid command format. Format: d ScaleA,ScaleB");
      error = true;
    }
  }
  





