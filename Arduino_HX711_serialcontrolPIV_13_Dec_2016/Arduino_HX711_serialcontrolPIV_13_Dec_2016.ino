// Setup load cell object
#include <HX711.h>
// scale(DT,SCK)
HX711 scaleA(A1, A0);
HX711 scaleB(A3, A2);

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
int pressureTime = 2000; //Change after testing pressurization 
unsigned long runTime = 0;
unsigned long resetTime = 0;
float printTime = 0;

bool path=true; // true is long track, false is short track (from control on LabVIEW panel)
int ValvePins[] = {triValveAPin, triValveBPin, valveA1Pin, valveA2Pin, valveB1Pin, valveB2Pin};

bool error = false;
// Initialize serial read variable
int incomingCom = 0;

void setup() {
  // Start Serial communications
  Serial.begin(9600);
  //Serial.println("start");
  for ( int thisPin = 0; thisPin <= 5; thisPin++) {
    pinMode(ValvePins[thisPin], OUTPUT);
  }
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
    // d scA.A Set settings
    // e Open valves
    // f Close valves
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
//     Serial.println(offset_B);
     Serial.println("Scale 1: ");
      Serial.println(scale_A, 4);
//      Serial.println("Scale 2: ");
//      Serial.println(scale_B, 4);
    }
    
    else if (incomingCom == 'd') {
      //Set settings
      settings();
    }
    
    
    else if(incomingCom == 'e'){
      //Open valves
      if(path) {
        //digitalWrite(triValveBPin, LOW);
        //digitalWrite(triValveAPin, HIGH);
        //delay (pressureTime);
        digitalWrite(valveB1Pin, HIGH);
        //delay(1000);
        //digitalWrite(valveA1Pin,HIGH);
        Serial.print(printTime);
        Serial.print("PUSH");
        Serial.println("  Long Path Open");
      }
      else if(!path) {
        //digitalWrite(triValveAPin, LOW);
        //digitalWrite(triValveBPin, HIGH);
        //delay (pressureTime);
        digitalWrite(valveA2Pin, HIGH);
        //delay(1000);
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
        digitalWrite(valveA1Pin, LOW);
        digitalWrite(valveB1Pin, LOW);
        digitalWrite(triValveAPin, LOW);
        Serial.print(printTime);
        Serial.print("PUSH");
        Serial.println("   Long Path Closed");
      }
      else if (!path) {
        digitalWrite(valveB2Pin, LOW);
        digitalWrite(valveA2Pin, LOW);
        digitalWrite(triValveBPin, LOW);
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
  //Serial.println((reading - offset_A) / scale_A, 4);
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
  
/*  
  while (Serial.available()) Serial.read();
  while(!Serial.available()) {
  scaleB.set_gain(128);
  float reading = scaleB.read_average(repeats);
  offset_B = reading;
  }
  Serial.println(offset_B);
  */
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

  //Initialize variables needed for calculations/reading
  int settingnum = 0;
  int counterA = 1;
  bool switchA = false;
  int error_code = 0;

  //settings--need to get 2 numbers with command
  //i.e. scA.A
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
      else {
        break; //discard extra characters, handle the case where last comma is omitted (standard practice)
      }
    }
    else if (incoming == '-' || ' '){
      continue;
    }
    else if (incoming == ',') {
      settingnum++;
    }
    else if (incoming == '.') {
        if (settingnum == 0) {
          switchA = true;
        }
      }
    
      else {
       error = true;
       error_code = incoming;
     }
    }
   
  if (scA1  && !error) { 
    scale_A = scA1 + scA2 / counterA;
      Serial.print(printTime);
      Serial.print("PUSH");
      Serial.println("   Settings complete.");
  }

    else {
      Serial.print(printTime);
      Serial.print(" ");
      Serial.print(error_code);
      Serial.print(scA1);
      Serial.print(scA2);
      Serial.println("   Settings failed due to invalid command format. Format: d ScaleA");
      error = true;
    }
  }





