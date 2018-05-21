// Setup load cell object
#include <HX711.h>
// scale(DT,SCK)
HX711 scaleA(A1, A0);
HX711 scaleB(A3, A2);
HX711 LoadCells[] = {scaleA, scaleB};

// Calibration variables for scale
long offset_A = 0;
long offset_B = 0;
float scale_A = 1;
float scale_B = 1;
int valve1Pin = 2;
unsigned long runTime = 0;
unsigned long resetTime = 0;

// Initialize serial read variable
int incomingCom = 0;

void setup() {
  // Start Serial communications
  Serial.begin(9600);
  pinMode(valve1Pin,OUTPUT);
  Serial.println("started");
}

void loop() {
  // put your main code here, to run repeatedly:
/*
  //If nothing else is happening, return the masses and the timestamp
  runTime = millis() - resetTime;
  Serial.print(float(runTime / float(1000)));
  Serial.print("     ");
  Serial.print(" A: "); //label output
  readCell1(10);
  Serial.print("    B: ");
  readCell2(10);
*/
}

void serialEvent() {
  //Whenever the Arduino receives a serial command, react
    incomingCom = Serial.read(); //Get the first character of the command


    //List of commands:
    // a Tare
    // b Wt1,Wt2 Calibrate scale A
    // c Output current settings
    // d offA,offB,scA.A,scB.B Set settings
    // e open valve
    // f close valve
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
      Serial.println(scale_A, 2);
      Serial.println("Scale 2: ");
      Serial.println(scale_B, 2);
     
    }
    
    else if (incomingCom == 'd') {
      //Set settings
      settings();
    }
    
    
    else if(incomingCom == 'e'){
      //Open valve through LabVIEW
      
      digitalWrite(valve1Pin,HIGH);
      Serial.println("open");
      
    }
    
    else if(incomingCom == 'f'){
      //Close valve through LabVIEW
      
      digitalWrite(valve1Pin, LOW);
      Serial.println("closed");
      
    }
   
    else if(incomingCom == 'g'){
     
      resetTime = millis();  
    }
    
    else if(incomingCom == 'h'){
      calibrationRead(false);
    }
    else {
      Serial.println("N");
    }
}


int readCell1(int repeats) {
  scaleA.set_gain(128);
  float reading = scaleA.read_average(repeats);
  Serial.print((reading - offset_A) / scale_A, 2);
  return 0;
}

int readCell2(int repeats) {
  scaleB.set_gain(128);
  float reading = scaleB.read_average(repeats);
  Serial.println((reading - offset_B) / scale_B, 2);
  return 0;
}

int tareCells(int repeats) {
  scaleA.set_gain(128);
  float reading = scaleA.read_average(repeats);
  offset_A = reading;
  scaleB.set_gain(128);
  reading = scaleB.read_average(repeats);
  offset_B = reading;
  return 0;
}

int calibrationRead(bool scaleIdent) {
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
    Serial.println("Calibration complete.");
    return 0;
  }
  
    else {
    Serial.println("Calibration failed due to invalid weight entry. Format: B Wt1,Wt2");
    return 1;
    }
  }


int calibration(int repeats, int weight1, int weight2, bool scaleIdent) {
  int readingA1 = 0;
  int readingA2 = 0;
  int readingB1 = 0;
  int readingB2 = 0;
  
  while (Serial.available()) Serial.read();
  
 // while (!Serial.available()); //wait for any command to be sent
  if (scaleIdent) {
    scaleA.set_gain(128);
    long readingA1 = scaleA.read_average(repeats);
    
  }
  else {
    scaleB.set_gain(128);
    long readingB1 = scaleB.read_average(repeats);
  }

  while (Serial.available()) Serial.read();
 
 // while (!Serial.available()); //wait for any command to be sent
  if (scaleIdent) {
    scaleA.set_gain(128);
    long readingA2 = scaleA.read_average(repeats);
    
  }
  else {
    scaleB.set_gain(128);
    long readingB2 = scaleB.read_average(repeats);
  }

  scale_A = (readingA2 - readingA1) / (0.5 * (weight2 - weight1));
  scale_B = (readingB2 - readingB1) / (0.5 * (weight2 - weight1));
  return 0;
}

int settings() {
  //Initialize storage variables for inputs
  long offA = 0;
  int offB = 0;
  double scA1 = 0; //before decimal point
  double scA2 = 0; //after decimal point
  float scB1 = 0; //before decimal point
  float scB2 = 0; //after decimal point

  //Initialize variables needed for calculations/reading
  int settingnum = 0;
  bool offAneg = false;
  bool offBneg = false;
  int counterA = 1;
  int counterB = 1;
  bool switchA = false;
  bool switchB = false;

  //Keep track of any errors
  bool error = false;
  
  //settings--need to get 4 numbers with command
  //i.e. offA,offB,scA.A,scB.B
  //Scales can have decimals, offset can't
  while (Serial.available()) {
    int incoming = Serial.read();
    if (incoming > 47 && incoming < 58) {
      if (settingnum == 0) {
        offA = (offA * 10) + (incoming - 48); //offsetA
      }
      else if (settingnum == 1) {
        offB = (offB * 10) + (incoming - 48); //offsetB
      }
      else if (settingnum == 2) {
        if (switchA) {
          scA2 = (scA2 * 10) + (incoming - 48);
          counterA = counterA * 10; //decimal part
        }
        else {
          scA1 = (scA1 * 10) + (incoming - 48); //whole number part
        }
      }
      else if (settingnum == 3) {
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
    else if (incoming == ',') {
      settingnum++;
    }
    else if (incoming == '.') {
      if (settingnum >= 1) {
        if (settingnum == 3) {
          switchB = true;
        }
        if (settingnum == 1) {
          switchA = true;
        }
      }
      else {
        error = true;
      }
    }
    else if (incoming == '-') {
      if (settingnum < 1) {
        if (offA) {
          offBneg = true;
        }
        else {
          offAneg = true;
        }
      }
    }
  }
  if (scA1 && scB1 && !error) { 
    //handle positive versus negative cases for offset
    if (offAneg) {
      offset_A = -1*offA;
    }
    else {
      offset_A = offA;
    }
    if (offBneg) {
     offset_B = -1*offB;
    }
    else {
      offset_B = offB;
    }
    scale_A = scA1 + scA2 / counterA;
    scale_B = scB1 + scB2 / counterB;
  
      Serial.println("Settings complete.");
      return 0;
  }

    else {
      Serial.println("Settings failed due to invalid command format. Format: D offA,offB,scA.A,scB.B");
    }
    return 1;
  }





