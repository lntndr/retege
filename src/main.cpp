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

// global variables
/// relay
const int relayPin = 13;
/// buzzer
const int buzPin = 19;
// objects
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);
Bounce * input = new Bounce[NUM_INPUTS];

// functions declarations
bool pressHoldButton(int i);
float changeSpanLinearly(int j, float base);

void setup() {
  // local variables
  /// buttons
  const int m1Pin = 2, m2Pin = 3, startPin = 4, p1Pin = 5, p2Pin = 6;
  /// toggles
  const int focusPin = 14, linGeoPin = 15, singStriPin = 16;
  /// bounce2 aux array
  const int inputPins[NUM_INPUTS] = {m1Pin, m2Pin,
                                  p1Pin, p2Pin,
                                  startPin,
                                  focusPin, linGeoPin, singStriPin};

  // declarations
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

  // run things

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
  // local variables
  static unsigned long ringInterval = 0;
  static unsigned long displayInterval = 0;
  // 0 = linear single, 1 = geometric single, 2 = linear strip, 3 = geometric strip
  static int runningMode = LINEAR_SINGLE;
  static int newRunningMode = LINEAR_SINGLE;
  // 0 = setup, 1 = exposure, 2 = focus
  static int runningStatus = SETUP;
  static unsigned long lampFirstMillis = 0;
  static float baseTimeSpan = 10;
  static int stripNumber = 6;
  static float stripDuration = 5;
  static unsigned int geometricReason = 3;
  static int geometricIndex = 0;
  static float timerDisplay = 0;
  static long rollingTime = 0;

  // run things
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
      newRunningMode = LINEAR_SINGLE;
    } else {
      newRunningMode = LINEAR_STRIP;
    }
  } else {
    if (!input[SINGLE_STRIP_TOGGLE].read()) {
      newRunningMode = GEOMETRIC_SINGLE;
    } else {
      newRunningMode = GEOMETRIC_STRIP;
    }
  }
  if (runningMode != newRunningMode && runningStatus != 2) {
    lcd.clear();
    lampFirstMillis = 0;
    displayInterval = 0;
    ringInterval = 0;
    runningMode = newRunningMode;
    runningStatus = SETUP;
  }

  // switch between runningStatus
  switch (runningStatus) {
    case SETUP:
      digitalWrite(relayPin,LOW);

      // READ BUTTONS 
      // check if runningMode has to be changed
      if (input[START_BUTTON].rose()) {
        runningStatus = EXPOSURE;
      } else if (input[FOCUS_TOGGLE].read()) {
        runningStatus = FOCUS;
      }
      if (runningStatus != SETUP) {
        rollingTime=timerDisplay*1000;
        lampFirstMillis = millis();
        lcd.clear();
        break;
      }
      // check buttons
      switch (runningMode) {
        case LINEAR_SINGLE:
          timerDisplay = baseTimeSpan;
          for (int i = 0; i < 4; i++) {
            if (pressHoldButton(i)) {
              baseTimeSpan = changeSpanLinearly(i,baseTimeSpan);
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
          timerDisplay=baseTimeSpan+(stripDuration*stripNumber);
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
          timerDisplay=baseTimeSpan*pow(2,(1.*stripNumber)/(1.*geometricReason));
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
      
      // DISPLAY OUTPUT
      // display output
      lcd.setCursor(0,0);
      lcd.print("SETUP");
      lcd.setCursor(0,1);
      switch (runningMode) {
        case LINEAR_SINGLE:
          lcd.print(timerDisplay,1);
          break;
        case GEOMETRIC_SINGLE:
          lcd.print(baseTimeSpan,1);
          if (geometricIndex>=0) {
            lcd.print("+");
          }
          lcd.print(geometricIndex);
          lcd.print("/");
          lcd.print(geometricReason);
          lcd.print("=");
          lcd.print(timerDisplay,2);
          break;
        case LINEAR_STRIP:
          lcd.print(baseTimeSpan,1);
          lcd.print("+[");
          lcd.print(stripNumber);
          lcd.print("*");
          lcd.print(stripDuration,1);
          lcd.print("s]");
          break;
        case GEOMETRIC_STRIP:
          lcd.print(baseTimeSpan,1);
          lcd.print("+[");
          lcd.print(stripNumber);
          lcd.print("*1/");
          lcd.print(geometricReason);
          lcd.print("]");
          break;
      }

      break; // end SETUP case

    case EXPOSURE:
      // check if runningMode has to be changed
      if (input[START_BUTTON].rose() || rollingTime <= 0) {
        runningStatus = SETUP;
        ringInterval = 0;
        rollingTime = 0;
        lampFirstMillis = 0;
        lcd.clear();
        break;
      }
      digitalWrite(relayPin,HIGH);
      rollingTime = (lampFirstMillis+(timerDisplay*1000)-millis());
      // play ringer
      switch(runningMode) {
        case LINEAR_SINGLE:
        case GEOMETRIC_SINGLE:
          if (millis()>=lampFirstMillis+ringInterval*1000){ // buzz every second
            tone(buzPin, 987, 100);
            ringInterval++;
          }
          break;
        case LINEAR_STRIP:
          if (millis()>= lampFirstMillis - 250 + baseTimeSpan*1000 + stripDuration*1000*ringInterval ) {
            tone(buzPin, 659, 150);
          }
          if (millis()>= lampFirstMillis + baseTimeSpan*1000 + stripDuration*1000*ringInterval ) {
            tone(buzPin, 880, 150);
            ringInterval++;
          } 
          break;
        case GEOMETRIC_STRIP:
          if (millis()>= lampFirstMillis - 250 +
                         baseTimeSpan*1000*pow(2,(1.*ringInterval)/(1.*geometricReason))) {
            tone(buzPin, 659, 50);
          } 
          if (millis()>= lampFirstMillis +
                         baseTimeSpan*1000*pow(2,(1.*ringInterval)/(1.*geometricReason))) {
            tone(buzPin, 880, 150);
            ringInterval++;
          }
          break;
      }

      // display output
      lcd.setCursor(0,0);
      lcd.print("EXPOSURE");
      lcd.setCursor(0,1);
      if (runningMode > 1) {
        lcd.print(ringInterval);
          lcd.print("/");
          lcd.print(stripNumber);
          lcd.print("  ");
      }
      lcd.print(rollingTime/1000.,1);
      if ((int)log10(rollingTime)+1 != (int)log10(rollingTime+100)+1) {
        //clear the screen when the number of digit changes
        lcd.clear(); 
      }
      break; // end EXPOSURE case

    case FOCUS:
      // check if state is changed
      if (!input[FOCUS_TOGGLE].read()) {
        runningStatus = 0;
        lampFirstMillis = SETUP;
        ringInterval = 0;
        displayInterval = 0;
        lcd.clear();
        break;
      } 

      digitalWrite(relayPin,HIGH);
      // play ringer
      if (millis()>=lampFirstMillis+ringInterval*1000) { // buzz every second
        tone(buzPin, 987, 100);
        ringInterval++;
      }
      // display output
      lcd.setCursor(0,0);
      lcd.print("FOCUS");
      if (millis()>=lampFirstMillis+displayInterval*100) {
        lcd.setCursor(0,1);
        lcd.print((millis()-lampFirstMillis)/1000.,1);
        displayInterval++;
      }
      break;
  }

}

bool pressHoldButton(int i) {
  static unsigned long holdInterval = 0;
  const int holdMillis = 100;
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

float changeSpanLinearly(int j, float base) {
  float y, k;
  if (j < 2) { // subtraction
    k = pow(10,j-1)*-1;
  } else {
    k = pow(10,j-3);
  }
  if (base+k < 0) {
    y = 0;
  } else if (base+k > 1000) {
    y = 999.9;
  } else {
    y= base+k;
  }
  return y;
}