#include <Arduino.h>
#include <bounce2.h>
#include <Liquidcrystal.h>

#define NUM_INPUTS 8

// pins
/// buttons
const int m1Pin = 2, m2Pin = 3, startPin = 4, p1Pin = 5, p2Pin = 6;

/// relay
const int relayPin = 13;

/// toggles
const int focusPin = 14, linGeoPin = 15, singStriPin = 16;

/// buzzer
const int buzPin = 19;

// service variables
const int inputPins[NUM_INPUTS] = {m1Pin, m2Pin,
                                  p1Pin, p2Pin,
                                  startPin,
                                  focusPin, linGeoPin, singStriPin};
const int start = 4, focus = 5, linGeo = 6, singStri = 7;

const int holdMillis = 100;
unsigned int holdInterval = 0;

// state variables
int runningMode = 0;      //0 = linear single, 1 = geometric single, 2 = linear strip, 3 = geometric strip
int runningStatus = 0;    //0 = setup, 1 = run, 2 = focus
unsigned long lampFirstMillis = 0;
float baseTimeSpan = 10;
int stripNumber = 1;
float stripDuration = 1;
unsigned int geometricReason = 1;
int geometricIndex = 0;
float timerValue = 0;

float linStepSeconds = 1;

// objects
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);
Bounce * input = new Bounce[NUM_INPUTS];

// functions
bool checkButton(int i);
float changeSpanLinearly(int i);

void setup() {
  Serial.begin(9600);

  pinMode(relayPin, OUTPUT);
  pinMode(buzPin,   OUTPUT);

  pinMode(p1Pin,    INPUT);
  pinMode(p2Pin,    INPUT);
  pinMode(startPin, INPUT);
  pinMode(m1Pin,    INPUT);
  pinMode(m2Pin,    INPUT);

  pinMode(focusPin,    INPUT);
  pinMode(linGeoPin,   INPUT);
  pinMode(singStriPin, INPUT);

  for (int i = 0; i < NUM_INPUTS; i++) {
    input[i].attach(inputPins[i], INPUT);
    input[i].interval(25);
  }
  lcd.begin(16,2);
  lcd.print("main.cpp");
  delay(1500);
  lcd.clear();
}

void loop() {
  // update buttons
  for (int i = 0; i < NUM_INPUTS; i++) {
    input[i].update();
    if (input[i].rose()) {
      lcd.clear();
    }
  }
  // read runningMode
  if (!input[linGeo].read()) {
    if (!input[singStri].read()) {
      runningMode = 0;
    } else {
      runningMode = 1;
    }
  } else {
    if (!input[singStri].read()) {
      runningMode = 2;
    } else {
      runningMode = 3;
    }
  }
  // switch between runningStatus
  switch (runningStatus) {
    case 0:
      digitalWrite(relayPin,LOW);
      // runningMode independent
      if (checkButton(start)) {
        runningStatus = 1;
      } else if (input[focus].read()) {
        runningStatus = 2;
      }

      // runningMode dependent
      if (runningStatus != 0) {
        if (runningMode>1) {
          timerValue=0;
        }
        lampFirstMillis = millis();
        break;
      }

      switch (runningMode) {
        case 0:
          timerValue = baseTimeSpan;
          for (int i = 0; i < 4; i++) {
            if (checkButton(i)) {
              baseTimeSpan = changeSpanLinearly(i);
            }
          }
          break;

        case 1:
          timerValue = baseTimeSpan*pow(2,(1.*geometricIndex)/(1.*geometricReason));
          for (int i = 0; i < 3; i=i+2) {
            if (checkButton(i)) {
              if (geometricReason+(i-1) > 0) {
                geometricReason = geometricReason+(i-1);
              } else {
                geometricReason = 1;
              }
            }
          }
          for (int i = 1; i < 4; i=i+2) {
            if (checkButton(i)) {
              geometricIndex = geometricIndex+(i-2);
            }
          }
          break;

        case 2:
          for (int i = 0; i < 3; i=i+2) {
            if (checkButton(i)) {
              if (stripNumber+(i-1) > 0) {
                stripNumber = stripNumber+(i-1);
              } else {
                geometricReason = 1;
              }
            }
          }
          for (int i = 1; i < 4; i=i+2) {
            if (checkButton(i)) {
              stripDuration = stripDuration+((i-2.)*0.1);
            }
          }
          break;
        case 3:
          for (int i = 0; i < 3; i=i+2) {
            if (checkButton(i)) {
              if (stripNumber+(i-1) > 0) {
                stripNumber = stripNumber+(i-1);
              } else {
                geometricReason = 1;
              }
            }
          }
          for (int i = 1; i < 4; i=i+2) {
            if (checkButton(i)) {
              if (geometricReason+(i-2) > 0) {
                geometricReason = geometricReason+(i-2);
              } else {
                geometricReason = 1;
              }
            }
          }
          break;
      }
      break;

    case 1:
      // runningMode independent
      if (checkButton(start)) {
        runningStatus = 0;
        lampFirstMillis = 0;
      }
      digitalWrite(relayPin,HIGH);
      // shape output
      switch (runningMode) {
        case 0:
          break;
        case 1:
          break;
        case 2:
          if (timerValue==0) {
            timerValue = baseTimeSpan + (stripDuration*stripNumber);
          } else {

          }
          break;
        case 3:
          if (timerValue==0) {
            timerValue = baseTimeSpan*pow(2,(1.*stripNumber/(1.*geometricReason)));
          } else {

          }
          break;
      }
      break;

    case 2:
      // runningMode independent
      if (!input[focus].read()) {
        runningStatus = 0;
        lampFirstMillis = 0;
        holdInterval = 0;
      }
      digitalWrite(relayPin,HIGH);
      if (millis()>lampFirstMillis+holdInterval*1000){ // buzz every second
        tone(buzPin, 987, 100);
        holdInterval++;
      }
      break;
  }

  lcd.setCursor(0,0);
  lcd.print("MS");
  lcd.print(runningMode);
  lcd.print(runningStatus);
  lcd.print("ST");
  lcd.print(stripNumber);
  lcd.print(stripDuration,1);
  lcd.print("GM");
  lcd.print(geometricReason);
  lcd.print(geometricIndex);
  lcd.setCursor(0,1);
  lcd.print("BS");
  lcd.print(baseTimeSpan,1);
  lcd.print(" TV");
  lcd.print(timerValue,1);
}

bool checkButton(int i) {
  bool y;
  if(input[i].read() && input[i].duration()>holdInterval*(holdMillis)) {
    y = 1;
    holdInterval++;
  } else {
    y = 0;
  }
  if (input[i].fell()) {
    holdInterval=0;
  }
  return y;
}

float changeSpanLinearly(int j) {
  float y, k;
  if (j < 2) { // subtraction
    k = pow(10,j-1)*-1;
  } else {
    k = pow(10,j-3);
  }
  Serial.print(j);
  Serial.print(" ");
  Serial.print(k);
  Serial.print("\n");
  if (baseTimeSpan+k < 0) {
    y = 0;
  } else if (baseTimeSpan+k > 1000) {
    y = 999.9;
  } else {
    y= baseTimeSpan+k;
  }
  return y;
}
