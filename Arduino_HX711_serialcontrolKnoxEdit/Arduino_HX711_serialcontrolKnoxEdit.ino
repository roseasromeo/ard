// Setup load cell object
#include <HX711.h>
// scale(DT,SCK)
HX711 scale(A1, A0);
// Calibration variables for scale
long offset_A = 0;
long offset_B = 0;
float scale_A = 1;
float scale_B = 1;
int ledPin = 13;
int valve1Pin = 2;
unsigned long runTime = 0;
unsigned long resetTime = 0;

// Initialize serial read variable
int incomingCom = 0;

void setup() {
  // Start Serial communications
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:

  //If nothing else is happening, return the masses
  runTime = millis() - resetTime;
  Serial.print(float(runTime / float(1000)));
  Serial.print("     ");
  Serial.print(" A: "); //label output--likely remove later?
  readCellA(10);
  //Serial.print("B: "); //label output--likely remove later?
  //readCellB(10); //Will need to switch to reading two different HX711 to get accurate readings on two cells
  //Serial.println("");
  //delay(200);
}

void serialEvent() {
  //Whenever the Arduino receives a serial command, react
 // while (Serial.available()) {
    incomingCom = Serial.read(); //Get the first character of the command

    //Capital Letters are for use with the Serial Monitor (human-readable responses)
    //Lowercase Letters are for use with LabVIEW (machine-interpretable responses)
    //Labview expects ! as the beginning and @ as the end of responses

    //List of commands:
    // A/a Tare
    // B/b Wt1,Wt2 Calibrate (b not implemented)
    // C/c Output current settings
    // D/d offA,offB,scA.A,scB.B Set settings
    
    if (incomingCom == 'A') {
      //tare
      Serial.println("Taring");
      tareCells(50);
      Serial.println(offset_A);
      Serial.println(offset_B);
      Serial.println("Tare complete");
    }
    else if (incomingCom == 'a') {
      //labview-friendly tare
      Serial.println("!");
      tareCells(50);
      Serial.println("@");
    }
    else if (incomingCom == 'B') {
      //calibration--need to get two weights in same units with command
      //i.e. B Wt1,Wt2
      calibrationRead(false);
    }
    else if (incomingCom == 'b') {
      //labview-friendly calibration--need to get two weights in same units with command
      //i.e. B Wt1,Wt2
      Serial.println("!");
      calibrationRead(true);
    }
    else if (incomingCom == 'C') {
      //Print settings
      Serial.println("Settings");
      Serial.print("Offset A: ");
      Serial.println(offset_A);
      //Serial.print("Offset B: ");
      //Serial.println(offset_B);
      Serial.print("Scale A: ");
      Serial.println(scale_A, 2);
      //Serial.print("Scale B: ");
      //Serial.println(scale_B, 2);
    }
    else if (incomingCom == 'c') {
      //Print settings without labels (labview-friendly)
      Serial.println("!");
      Serial.println("Offset: ");
      Serial.println(offset_A);
     // Serial.println(offset_B);
     Serial.println("Scale: ");
      Serial.println(scale_A, 2);
      //Serial.println(scale_B, 2);
      Serial.println("@");
    }
    else if (incomingCom == 'D') {
      //Set settings
      settings(false);
    }
    else if (incomingCom == 'd') {
      //Set settings
      settings(true);
    }
    else if(incomingCom == 'E'){
      //Open valve through serial monitor
      digitalWrite(ledPin, HIGH);
      digitalWrite(valve1Pin, HIGH);
      Serial.println("open");
    }
    else if(incomingCom == 'e'){
      //Open valve through LabVIEW
      Serial.println("!");
      digitalWrite(ledPin, HIGH);
      digitalWrite(valve1Pin,HIGH);
      Serial.println("open");
      Serial.println("@");
    }
    else if(incomingCom == 'F'){
      //Close valve through serial monitor
      digitalWrite(ledPin, LOW);
      digitalWrite(valve1Pin, LOW);
      Serial.println("closed");
    }
    else if(incomingCom == 'f'){
      //Close valve through LabVIEW
      Serial.println("!");
      digitalWrite(ledPin, LOW);
      digitalWrite(valve1Pin, LOW);
      Serial.println("closed");
      Serial.println("@");
    }
    else if(incomingCom == 'G'){
      resetTime = millis();
      Serial.println("Time reset");
    }
    else if(incomingCom == 'g'){
      Serial.println("!");
      resetTime = millis();
      Serial.println("@");
      
    }
    else {
      Serial.println("N");
    }
  //}
}

int readCellA(int repeats) {
  scale.set_gain(128);
  float reading = scale.read_average(repeats);
  Serial.println((reading - offset_A) / scale_A, 2);
  return 0;
}

int readCellB(int repeats) {
  scale.set_gain(32);
  float reading = scale.read_average(repeats);
  Serial.println((reading - offset_B) / scale_B, 2);
  return 0;
}

int tareCells(int repeats) {
  scale.set_gain(128);
  float reading = scale.read_average(repeats);
  offset_A = reading;
  scale.set_gain(32);
  reading = scale.read_average(repeats);
  offset_B = reading;
  return 0;
}

int calibrationRead(bool LV) {
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
    calibration(50, weight1, weight2, LV);
    if (LV) {
      Serial.println("%"); //Success
      Serial.println("@");
    }
    else {
      Serial.println("Calibration complete.");
    }
    return 0;
  }
  else {
    if (LV) {
      Serial.println("#"); //Failure
      Serial.println("@");
    }
    else {
      Serial.println("Calibration failed due to invalid weight entry. Format: B Wt1,Wt2");
    }
    return 1;
  }
}

int calibration(int repeats, int weight1, int weight2, bool LV) {
  while (Serial.available()) Serial.read();
  if (LV) {
    Serial.println("$"); //Tell LabVIEW to take next step
  }
  else {
    Serial.println("Place weight 1 centered on scale and send command 1.");
  }
  while (!Serial.available()); //wait for any command to be sent
  scale.set_gain(128);
  long readingA1 = scale.read_average(repeats);
  if (!LV) Serial.println(readingA1 - offset_A);
  scale.set_gain(32);
  long readingB1 = scale.read_average(repeats);
  if (!LV) Serial.println(readingB1 - offset_B);

  while (Serial.available()) Serial.read();
    if (LV) {
    Serial.println("$"); //Tell LabVIEW to take next step
  }
  else {
    Serial.println("Place weight 1 centered on scale and send command 2.");
  }
  while (!Serial.available()); //wait for any command to be sent
  scale.set_gain(128);
  long readingA2 = scale.read_average(repeats);
  if (!LV) Serial.println(readingA2 - offset_A);
  scale.set_gain(32);
  long readingB2 = scale.read_average(repeats);
  if (!LV) Serial.println(readingB2 - offset_B);

  scale_A = (readingA2 - readingA1) / (0.5 * (weight2 - weight1));
  scale_B = (readingB2 - readingB1) / (0.5 * (weight2 - weight1));
  return 0;
}

int settings(bool LV) {
  if (LV) Serial.println("!");

  //Initialize storage variables for inputs
  long offA = 0;
  //int offB = 0;
  double scA1 = 0; //before decimal point
  double scA2 = 0; //after decimal point
  //float scB1 = 0; //before decimal point
  //float scB2 = 0; //after decimal point

  //Initialize variables needed for calculations/reading
  int settingnum = 0;
  bool offAneg = false;
  //bool offBneg = false;
  int counterA = 1;
  //int counterB = 1;
  bool switchA = false;
  //bool switchB = false;

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
      //else if (settingnum == 1) {
        //offB = (offB * 10) + (incoming - 48); //offsetB
      //}
      else if (settingnum == 1) {
        if (switchA) {
          scA2 = (scA2 * 10) + (incoming - 48);
          counterA = counterA * 10; //decimal part
        }
        else {
          scA1 = (scA1 * 10) + (incoming - 48); //whole number part
        }
      }
      //else if (settingnum == 3) {
        //if (switchB) {
          //scB2 = (scB2 * 10) + (incoming - 48); //decimal part
          //counterB = counterB * 10;
        //}
        //else {
          //scB1 = (scB1 * 10) + (incoming - 48); //whole number part
        //}
      //}
      else {
        break; //discard extra characters, handle the case where last comma is omitted (standard practice)
      }
    }
    else if (incoming == ',') {
      settingnum++;
    }
    else if (incoming == '.') {
      if (settingnum >= 1) {
        //if (settingnum == 3) {
          //switchB = true;
        //}
        if (settingnum == 1) {
          switchA = true;
        }
      }
      else {
        if (!LV) Serial.println("Offsets cannot have decimal values.");
        error = true;
      }
    }
    else if (incoming == '-') {
      if (settingnum < 1) {
        //if (offA) {
          //offBneg = true;
        //}
       // else {
          offAneg = true;
        //}
      }
      else {
        if (!LV) Serial.println("Scales cannot be negative. Used positive value.");
      }
    }
  }
  if (scA1 && !error) { //&&scB1
    //handle positive versus negative cases for offset
    if (offAneg) {
      offset_A = -1*offA;
    }
    else {
      offset_A = offA;
    }
    //if (offBneg) {
      //offset_B = -1*offB;
    //}
    //else {
      //offset_B = offB;
    //}
    scale_A = scA1 + scA2 / counterA;
    //scale_B = scB1 + scB2 / counterB;
    if (LV) {
      Serial.println("%"); //Success
      Serial.println("@");
    }
    else {
      Serial.println("Settings complete.");
    }
    return 0;
  }
  else {
    if (LV) {
      Serial.println("#"); //Failure
      Serial.println("@");
    }
    else {
      Serial.println("Settings failed due to invalid command format. Format: D offA,offB,scA.A,scB.B");
    }
    return 1;
  }
}


/// Hopes and dreams
// Toggle absolute reading and offset/scaled reading with command
// Save Settings to file before starting to record
// Comment better
// Valve trigger and output exactly when to the file

