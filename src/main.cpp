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
float timeSpan = 10;
int stripNumber = 1;
unsigned int geometricReason = 1;
float linStepSeconds = 1;

// objects
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);
Bounce * input = new Bounce[NUM_INPUTS];

// functions
bool checkButton(int i);
float changeSpanLinearly(int i);
float changeSpanGeometrically(int i, int q);

void setup() {
  Serial.begin(9600);
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
      // runningMode independent
      if (checkButton(start)) {
        runningStatus = 1;
        lampFirstMillis = millis();
      } else if (input[focus].read()) {
        runningStatus = 2;
        lampFirstMillis = millis();
      }
      // runningMode dependent
      switch (runningMode) {
        case 0:
          for (int i = 0; i < 4; i++) {
            if (checkButton(i)) {
              timeSpan = changeSpanLinearly(i);
            }
          }
          break;
        case 1:
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
              timeSpan = changeSpanGeometrically(i-2,geometricReason);
            }
          }
          break;
        case 3:
          break;
        case 4:
          break;
      }
      break;
    case 1:
      // runningMode independent
      if (checkButton(start)) {
        runningStatus = 0;
        lampFirstMillis = 0;
      }
      break;
    case 2:
      // runningMode independent
      if (!input[focus].read()) {
        runningStatus = 0;
        lampFirstMillis = 0;
      }
      break;
  }

  lcd.setCursor(0,0);
  lcd.print(runningMode);
  lcd.setCursor(2,0);
  lcd.print(runningStatus);
  lcd.setCursor(0,1);
  lcd.print(stripNumber);
  lcd.setCursor(2,1);
  lcd.print(geometricReason);
  lcd.setCursor(5,1);
  lcd.print(timeSpan);

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
  if (timeSpan+k < 0) {
    y = 0;
  } else if (timeSpan+k > 1000) {
    y = 999.9;
  } else {
    y= timeSpan+k;
  }
  return y;
}

float changeSpanGeometrically(int i, int q) {
  float y;
  Serial.print(q);
  Serial.print(" ");
  Serial.print(i);
  y = timeSpan*(1./q)*i;
  Serial.print(" ");
  Serial.print(y);
  Serial.print("\n");
  y = timeSpan+y;
  return y;
}