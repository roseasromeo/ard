//Credit to Katherine Knox for developing the initial framework for the Arduino-LabVIEW communication

//If two scales and full piping, set to true, uncomment HX711 line
bool full_setup = false;

// Setup load cell object
#include <HX711.h>
// scale(DT,SCK)
HX711 scaleA(A1, A0);
//if (full_setup) 
HX711 scaleB(A3, A2); //will this conflict with using A3 and A2 in not full_setup? Not sure

// Setup DAC board
#include <Wire.h>
#include <Adafruit_MCP4725.h>

Adafruit_MCP4725 dac;

// Calibration variables for scale
long offset_A = 0;
float scale_A = 1;
// for full_setup 
long offset_B = 0;
float scale_B = 1;

// Pin assignments: valves
int valveA1Pin = 2;
int valveA2Pin = 3;
int valveB1Pin = 4; //A1 goes to B1 and A2 goes to B2
int valveB2Pin = 5; //A1 to B1 is long track, B2 to A2 is short track
int triValveAPin = 6;
int triValveBPin = 7;
bool path=true; // true is long track, false is short track (from control on LabVIEW panel)
int ValvePins[6];
int number_valves;

// Pin assignments: pressure regulators/gauges
// Pins A4 and A5 reserved for I2C to DAC board
int regulatorA_out_pin = A3; //will cause problems in full_setup, need Nano
int gaugeA_pin = A2; //will cause problems in full_setup, need Nano
int regulatorB_out_pin = 0; //need Nano
int gaugeB_pin = 0; //need Nano

// int camera_pin = 12;

// Pressure Settings
float pressure = 0;
float p_max = 100;
float p_min = 0;
bool go_to_zero = false;

// Timing
int pressureTime = 2000; //Change after testing pressurization 
unsigned long runTime = 0;
unsigned long resetTime = 0;
float printTime = 0;

// No errors at start
bool error = false;

// Initialize serial read variable
int incomingCom = 0;

void setup() {
  // Start Serial communications
  Serial.begin(9600);
  Serial.print("PUSH");
  Serial.println("start");
  if (full_setup) {
    int temp[] = {triValveAPin, triValveBPin, valveA1Pin, valveA2Pin, valveB1Pin, valveB2Pin};
    for (int thisPin = 0; thisPin <= (number_valves - 1); thisPin++) {
      ValvePins[thisPin] = temp[thisPin];
    }
   
    number_valves = 6;
  }
  else {
    ValvePins[0] = {valveA1Pin};
    number_valves = 1;
  }
  for ( int thisPin = 0; thisPin <= (number_valves - 1); thisPin++) {
    pinMode(ValvePins[thisPin], OUTPUT);
  }
  pinMode(regulatorA_out_pin, INPUT);
  pinMode(gaugeA_pin, INPUT);
  if (full_setup) {
    pinMode(gaugeB_pin, INPUT);
    pinMode(regulatorB_out_pin, INPUT);
  }
  
  // Start communicating with DAC board
  dac.begin(0x60);
  
  //pinMode(camera_pin) = OUTPUT;
}
  


void loop() {
  // put your main code here, to run repeatedly:
  
  if(error == true){
    for ( int thisPin = 0; thisPin <= (number_valves - 1); thisPin++) {
      digitalWrite(ValvePins[thisPin], LOW);  
    }
    dac.setVoltage(0,false);
    Serial.println("Error");
  }
  //If nothing else is happening, return the masses and the timestamp, set the pressure
 else{
  runTime = millis() - resetTime;
  printTime = float(runTime/float(1000));
  Serial.print(printTime);
  Serial.print("     ");
  Serial.print(" A: "); //label output
  readCell(10, true);
  if (full_setup) {
    Serial.print("    B: ");
    readCell(10, false);
  }
  Serial.print("    P: ");
  readGauge(10);
  Serial.print("    R: ");
  readRegulator(10);
  Serial.println("");
  
  // Set Pressure via DAC 
  if (go_to_zero) {
    dac.setVoltage(0, false);
  }
  else {
    dac.setVoltage(convert_p_dac(pressure), false);
  }
 }
}

void serialEvent() {
  //Whenever the Arduino receives a serial command, react
    incomingCom = Serial.read(); //Get the first character of the command


    //List of commands:
    // a Tare
    // b wt1,wt2 Calibrate scale A
    // c Output current settings
    // d scA.A Set settings
    // e Open valves
    // f Close valves
    // g Reset time
    // h wt1,wt2 Calibrate scale B
    // j press Set Pressure
  
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
      if (full_setup) {
        Serial.println(offset_B);
      }
      Serial.println("Scale 1: ");
      Serial.println(scale_A, 4);
      if (full_setup) {
        Serial.println("Scale 2: ");
        Serial.println(scale_B, 4);
      }
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
    
    else if(incomingCom == 'j'){
      pressureSet();
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


void readCell(int repeats, bool scaleIdent) {
  if (scaleIdent) {
    scaleA.set_gain(128);
    float reading = scaleA.read_average(repeats);
    Serial.print((reading - offset_A) / scale_A, 4);
  }
  else {
    scaleB.set_gain(128);
    float reading = scaleB.read_average(repeats);
    Serial.print((reading - offset_B) / scale_B, 4);
  }
}



void tareCells(int repeats) {
   while (Serial.available()) Serial.read();
  
  while (!Serial.available()) {
    scaleA.set_gain(128);
    float reading = scaleA.read_average(repeats);
    offset_A = reading;
  }
  Serial.println(offset_A);
  
  if (full_setup) {  
    while (Serial.available()) Serial.read();
    while(!Serial.available()) {
      scaleB.set_gain(128);
      float reading = scaleB.read_average(repeats);
      offset_B = reading;
    }
    Serial.println(offset_B);
  }
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
    delay(2);
  }

  if (scA1 && (!full_setup || scB1) && !error) { 
    scale_A = scA1 + scA2 / counterA;
    scale_B = scB1 + scB2 / counterB;
    Serial.print(printTime);
    Serial.print("PUSH");
    Serial.println("   Settings complete.");
  }
  else {
    Serial.print(printTime);
    if (full_setup) {
      Serial.println("   Settings failed due to invalid command format. Format: d ScaleA.A,ScaleB.B");
    }
    else {
      Serial.println("   Settings failed due to invalid command format. Format: d ScaleA.A");
    }
    error = true;
  }
}

void pressureSet() {
  // Take in the pressure in psi from the serial monitor
  pressure = 0;
  while (Serial.available()) {
    int incoming = Serial.read();
    if (incoming > 47 && incoming < 58) {
      pressure = (pressure * 10) + (incoming - 48);
      go_to_zero = false;
    }
    else if (incoming == '-') {
      pressure = 0;
      go_to_zero = true;
    }
    delay(2);
  }
  Serial.print("PUSH");
  Serial.print("   Pressure Set:" );
  Serial.println(pressure);
}

void readGauge(int repeats) {
  // Read in voltage from the gauge and convert to pressure
  int reading_sum = 0;
  int gaugepin;
  
  if (full_setup) {
    if (path) {
      gaugepin = gaugeA_pin;
    }
    else {
      gaugepin = gaugeB_pin;
    }
  }
  else {
    gaugepin = gaugeA_pin;
  }
  for (int i = 0; i < repeats; i++) {
    reading_sum = reading_sum + analogRead(gaugepin);
  }
  float reading = (float)reading_sum/(float)repeats;
  Serial.print(((reading * 5.0/1024.0) - 1.0) * (p_max-p_min)/4.0, 4);
}

int convert_p_dac(float pressure){
  float slope = 25.907;
  float intercept = 652.61;
  
  return round(slope*pressure + intercept);
}


