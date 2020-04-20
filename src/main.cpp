#include <Arduino.h>
#include <bounce2.h>
#include <Liquidcrystal.h>

#define NUM_INPUTS 8

#define LINEAR_SINGLE 0
#define GEOMETRIC_SINGLE 1
#define LINEAR_STRIP 2
#define GEOMETRIC_STRIP 3

#define SETUP 0
#define EXPOSURE 1
#define FOCUS 2

#define START_BUTTON 4
#define FOCUS_TOGGLE 5
#define LINEAR_GEOMETRIC_TOGGLE 6
#define SINGLE_STRIP_TOGGLE 7

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
const int holdMillis = 100;
unsigned long holdInterval = 0;

// state variables

// 0 = linear single, 1 = geometric single, 2 = linear strip, 3 = geometric strip
int runningMode = LINEAR_SINGLE;
// 0 = setup, 1 = exposure, 2 = focus
int runningStatus = SETUP;
unsigned long lampFirstMillis = 0;
float baseTimeSpan = 10;
int stripNumber = 1;
float stripDuration = 1;
unsigned int geometricReason = 1;
int geometricIndex = 0;
float timerDisplay = 0;
long rollingTime = 0;
float linStepSeconds = 1;

// objects
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);
Bounce * input = new Bounce[NUM_INPUTS];

// functions
bool pressHoldButton(int i);
float changeSpanLinearly(int i);
void debugPrint();

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
    input[i].interval(40);
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
  if (!input[LINEAR_GEOMETRIC_TOGGLE].read()) {
    if (!input[SINGLE_STRIP_TOGGLE].read()) {
      runningMode = LINEAR_SINGLE;
    } else {
      runningMode = LINEAR_STRIP;
    }
  } else {
    if (!input[SINGLE_STRIP_TOGGLE].read()) {
      runningMode = GEOMETRIC_SINGLE;
    } else {
      runningMode = GEOMETRIC_STRIP;
    }
  }

  // switch between runningStatus
  switch (runningStatus) {
    case SETUP:
      digitalWrite(relayPin,LOW);
      // runningMode independent
      if (input[START_BUTTON].rose()) {
        runningStatus = EXPOSURE;
        Serial.print(timerDisplay);
      } else if (input[FOCUS_TOGGLE].read()) {
        runningStatus = FOCUS;
      }

      // runningMode dependent
      if (runningStatus != SETUP) {
        switch(runningMode) {
          case LINEAR_STRIP:
            timerDisplay=baseTimeSpan+(stripDuration*stripNumber);
            break;
          case GEOMETRIC_STRIP:
            timerDisplay=baseTimeSpan*pow(2,(1.*stripNumber)/(1.*geometricReason));
            break;
          }
        rollingTime=timerDisplay*1000;
        lampFirstMillis = millis();
        break;
      }

      switch (runningMode) {
        case LINEAR_SINGLE:
          timerDisplay = baseTimeSpan;
          for (int i = 0; i < 4; i++) {
            if (pressHoldButton(i)) {
              baseTimeSpan = changeSpanLinearly(i);
            }
          }
          break;

        case GEOMETRIC_SINGLE:
          timerDisplay = baseTimeSpan*pow(2,(1.*geometricIndex)/(1.*geometricReason));
          for (int i = 0; i < 3; i=i+2) {
            if (pressHoldButton(i)) {
              if (geometricReason+(i-1) > 0) {
                geometricReason = geometricReason+(i-1);
              } else {
                geometricReason = 1;
              }
            }
          }
          for (int i = 1; i < 4; i=i+2) {
            if (pressHoldButton(i)) {
              geometricIndex = geometricIndex+(i-2);
            }
          }
          break;

        case LINEAR_STRIP:
          for (int i = 0; i < 3; i=i+2) {
            if (pressHoldButton(i)) {
              if (stripNumber+(i-1) > 0) {
                stripNumber = stripNumber+(i-1);
              } else {
                geometricReason = 1;
              }
            }
          }
          for (int i = 1; i < 4; i=i+2) {
            if (pressHoldButton(i)) {
              stripDuration = stripDuration+((i-2.)*0.1);
            }
          }
          break;
        case GEOMETRIC_STRIP:
          for (int i = 0; i < 3; i=i+2) {
            if (pressHoldButton(i)) {
              if (stripNumber+(i-1) > 0) {
                stripNumber = stripNumber+(i-1);
              } else {
                geometricReason = 1;
              }
            }
          }
          for (int i = 1; i < 4; i=i+2) {
            if (pressHoldButton(i)) {
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

    case EXPOSURE:
      // runningMode independent -> time scaling and button reading
      if (input[START_BUTTON].rose() || rollingTime <= 0) {
        runningStatus = SETUP;
        holdInterval = 0;
        rollingTime = 0;
        lampFirstMillis = 0;
        break;
      }
      digitalWrite(relayPin,HIGH);
      rollingTime = (lampFirstMillis+(timerDisplay*1000)-millis());

      // runningMode dependent -> ringer and display
      switch(runningMode) {
        case LINEAR_SINGLE:
        case GEOMETRIC_SINGLE:
          if (millis()>=lampFirstMillis+holdInterval*1000){ // buzz every second
            tone(buzPin, 987, 100);
            holdInterval++;
          }
          break;
        case LINEAR_STRIP:
          if (millis()>= lampFirstMillis - 250 + baseTimeSpan*1000 + stripDuration*1000*holdInterval ) {
            tone(buzPin, 659, 150);
          }
          if (millis()>= lampFirstMillis + baseTimeSpan*1000 + stripDuration*1000*holdInterval ) {
            tone(buzPin, 880, 150);
            holdInterval++;
          } 
          break;
        case GEOMETRIC_STRIP:
          if (millis()>= lampFirstMillis - 250 +
                         baseTimeSpan*1000*pow(2,(1.*holdInterval)/(1.*geometricReason))) {
            tone(buzPin, 659, 50);
          } 
          if (millis()>= lampFirstMillis +
                         baseTimeSpan*1000*pow(2,(1.*holdInterval)/(1.*geometricReason))) {
            tone(buzPin, 880, 150);
            holdInterval++;
          }
          break;
      }

      break;

    case FOCUS:
      // runningMode independent
      if (!input[FOCUS_TOGGLE].read()) {
        runningStatus = 0;
        lampFirstMillis = SETUP;
        holdInterval = 0;
        break;
      }
      digitalWrite(relayPin,HIGH);
      if (millis()>=lampFirstMillis+holdInterval*1000){ // buzz every second
        tone(buzPin, 987, 100);
        holdInterval++;
      }
      break;
  }

  // shish
  debugPrint();
}

bool pressHoldButton(int i) {
  bool y;
  if(input[i].read() && input[i].duration()>=holdInterval*(holdMillis)) {
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
  if (baseTimeSpan+k < 0) {
    y = 0;
  } else if (baseTimeSpan+k > 1000) {
    y = 999.9;
  } else {
    y= baseTimeSpan+k;
  }
  return y;
}

void debugPrint() {
  lcd.setCursor(0,0);
  lcd.print(runningStatus);
  lcd.print(runningMode);
  lcd.print("  ");
  lcd.print(stripDuration);
  lcd.print(stripNumber);
  lcd.print("  ");
  lcd.print(geometricIndex);
  lcd.print(geometricReason);
  lcd.setCursor(0,1);
  lcd.print(rollingTime/1000.,1);
  lcd.print("  ");
  lcd.print(baseTimeSpan);
}
