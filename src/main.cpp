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
const int inputPins[NUM_INPUTS] = {p1Pin, p2Pin,
                          m1Pin, m2Pin,
                          startPin,
                          focusPin, linGeoPin, singStriPin};
const int p1 = 0, p2 = 1, m1 = 2, m2 = 3, start = 4, focus = 5, linGeo = 6, singStri = 7;
const int holdMillis = 250;
int holdInterval = 0;

// state variables
int runningMode = 0;      //0 = linear single, 1 = geometric single, 2 = linear strip, 3 = geometric strip
int runningStatus = 0;    //0 = setup, 1 = run, 2 = focus
unsigned long lampFirstMillis = 0;

// objects
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);
Bounce * input = new Bounce[NUM_INPUTS];

// functions
bool checkButton(int i);

void setup() {
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
  for (size_t i = 1; i < NUM_INPUTS; i++) {
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
      if (checkButton(start)) {
        runningStatus = 1;
        lampFirstMillis = millis();
      } else if (input[focus].read()) {
        runningStatus = 2;
        lampFirstMillis = millis();
      }
      break;
    case 1:
      if (checkButton(start)) {
        runningStatus = 0;
        lampFirstMillis = 0;
      }
      break;
    case 2:
      if (!input[focus].read()) {
        runningStatus = 0;
        lampFirstMillis = 0;
      }
      break;
  }

  lcd.setCursor(0,0);
  lcd.print(runningMode);
  lcd.setCursor(2,0);
  lcd.print(lampFirstMillis);
  lcd.setCursor(0,1);
  lcd.print(runningStatus);
}

bool checkButton(int i) {
  bool y;
  if(input[i].read() && input[i].duration()>holdInterval*holdMillis) {
    y = true;
    holdInterval++;
  } else {
    y = false;
  }
  if(input[i].fell()) {
    holdInterval=0;
  }
  return y;
}