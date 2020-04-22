#include <Arduino.h>
#include <Bounce2.h>
#include <LiquidCrystal.h>

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
Bounce * input = new Bounce[8];

// functions declarations
bool pressHoldButton(int i);
float changeSpanLinearly(int j, float base);
unsigned long stripRinger(byte mode, unsigned long lampRefer, float stripDuration, float baseTimeSpan, unsigned long ringCount, byte reason);
unsigned long metronome(unsigned long lampRefer, unsigned long ringCount);
byte updateRunningMode(byte runningMode, bool linGeo, bool sinStrip);
byte updateRunningStatus(byte runningStatus, bool & runningStatusChanged, bool startRose, bool focusHigh, unsigned long rollingTime);
unsigned long updateEvalTimeSpan(byte mode, unsigned long base, int index, byte reason, unsigned long duration, byte strip);
unsigned long updateBaseTimeSpan(unsigned long baseTimeSpan);

void setup() {
  // local variables
  /// buttons
  const int m1Pin = 2, m2Pin = 3, startPin = 4, p1Pin = 5, p2Pin = 6;
  /// toggles
  const int focusPin = 14, linGeoPin = 15, singStriPin = 16;
  /// bounce2 aux array
  const int inputPins[8] = {m1Pin, m2Pin,
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

  for (int i = 0; i < 8; i++) {
    input[i].attach(inputPins[i], INPUT);
    input[i].interval(40);
  }
  lcd.begin(16,2);
  lcd.print("retege v0.1");
  delay(2000);
  lcd.clear();
  Serial.begin(9600);
}

void loop() {
  // local variables
  static unsigned long ringCount = 0;
  // 0 = linear single, 1 = geometric single, 2 = linear strip, 3 = geometric strip
  static byte runningMode = LINEAR_SINGLE;
  // 0 = setup, 1 = exposure, 2 = focus
  static byte runningStatus = SETUP;
  static bool runningStatusChanged = 1;
  static unsigned long lampFirstMillis = 0;
  static unsigned long baseTimeSpan = 10000;
  static byte stripNumber = 6;
  static unsigned long stripDuration = 5000;
  static byte geometricReason = 3;
  static int geometricIndex = 0;
  static unsigned long evalTimeSpan = 0;
  static unsigned long rollingTime = 0;

  // update buttons
  for (int i = 0; i < 8; i++) {
    input[i].update();
    if (input[i].rose()) {
      lcd.clear();
    }
  }

  runningStatus = updateRunningStatus(runningStatus, runningStatusChanged, input[START_BUTTON].rose(), input[FOCUS_TOGGLE].read(), rollingTime);
  switch (runningStatus) {
    case SETUP:
      if (runningStatusChanged) {
        ringCount = 0;
        rollingTime = 0;
        lampFirstMillis = 0;
        lcd.clear();
      }
      // turn off the lamp
      digitalWrite(relayPin,LOW);
      runningMode = updateRunningMode(runningMode,input[LINEAR_GEOMETRIC_TOGGLE].read(),input[SINGLE_STRIP_TOGGLE].read());
      // update evalTimeSpangi
      evalTimeSpan = updateEvalTimeSpan(runningMode, baseTimeSpan, geometricIndex, geometricReason, stripDuration, stripNumber);
      /*
      geometricReason = updateGeometricReason();
      geometricIndex = updateGeometricIndex();
      stripNumber = updateStripNumber();
      stripDuration = updateStripDuration();
      */
      // read buttons
      switch (runningMode) {
        case LINEAR_SINGLE:
          baseTimeSpan = updateBaseTimeSpan(baseTimeSpan);
          break;

        case GEOMETRIC_SINGLE:
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
              stripDuration = stripDuration+((i-2)*100);
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
      
      // DISPLAY OUTPUT
      // display output
      lcd.setCursor(0,0);
      lcd.print("SETUP");
      lcd.setCursor(0,1);
      switch (runningMode) {
        case LINEAR_SINGLE:
          lcd.print(evalTimeSpan/1000.,1);
          break;
        case GEOMETRIC_SINGLE:
          lcd.print(baseTimeSpan/1000.,1);
          if (geometricIndex>=0) {
            lcd.print("+");
          }
          lcd.print(geometricIndex);
          lcd.print("/");
          lcd.print(geometricReason);
          lcd.print("=");
          lcd.print(evalTimeSpan/1000.,2);
          break;
        case LINEAR_STRIP:
          lcd.print(baseTimeSpan/1000.,1);
          lcd.print("+[");
          lcd.print(stripNumber);
          lcd.print("*");
          lcd.print(stripDuration/1000.,1);
          lcd.print("s]");
          break;
        case GEOMETRIC_STRIP:
          lcd.print(baseTimeSpan/1000.,1);
          lcd.print("+[");
          lcd.print(stripNumber);
          lcd.print("*1/");
          lcd.print(geometricReason);
          lcd.print("]");
          break;
      }

      break; // end SETUP case

    case EXPOSURE:
      if (runningStatusChanged) {
        rollingTime=evalTimeSpan;
        lampFirstMillis = millis() + 10; // 10 ms buffer allows to use unsigned long
        lcd.clear();
      }
      digitalWrite(relayPin,HIGH);
      rollingTime = (lampFirstMillis+(evalTimeSpan)-millis());
      // play ringer
      switch(runningMode) {
        case LINEAR_SINGLE:
        case GEOMETRIC_SINGLE:
          ringCount = metronome(lampFirstMillis,ringCount);
          break;
        case LINEAR_STRIP:
        case GEOMETRIC_STRIP:
          ringCount = stripRinger(runningMode, lampFirstMillis, stripDuration, baseTimeSpan, ringCount, geometricReason);
          break;
      }

      // display output
      lcd.setCursor(0,0);
      lcd.print("EXPOSURE");
      lcd.setCursor(0,1);
      if (runningMode > 1) {
        lcd.print(ringCount);
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
      if (runningStatusChanged) {
        lampFirstMillis = millis() + 10; // 10 ms buffer allows to use unsigned long
        lcd.clear();
      }
      digitalWrite(relayPin,HIGH);
      // play ringer
      ringCount = metronome(lampFirstMillis,ringCount);
      // display output
      lcd.setCursor(0,0);
      lcd.print("FOCUS");
      lcd.setCursor(0,1);
      if (millis()>=lampFirstMillis) {
        lcd.print((millis()-lampFirstMillis)/1000.,1);
      }
      break;
  }
}

bool pressHoldButton(int i) {
  static unsigned long holdInterval = 0;
  static int holdMillis = 100;
  bool y;

  if(input[i].read() && input[i].duration()>=holdInterval*(holdMillis)) {
    y = 1;
    holdInterval++;
  } else {
    y = 0;
  }

  for (byte k = 1; k < 5; k++) {
    if (input[i].read() && input[i].duration()>k*1000) {
      holdMillis = 100-(k*20);
    }
  }

  if (input[i].fell()) {
    holdInterval=0;
    holdMillis = 100;
  }
  return y;
}

float changeSpanLinearly(int j, float base) {
  float y, k;
  if (j < 2) { // subtraction
    k = pow(10,j+2)*-1;
  } else {
    k = pow(10,j);
  }
  if (base+k < 0) {
    y = 0;
  } else if (base+k >= 1000000) {
    y = 999999;
  } else {
    y= base+k;
  }
  return y;
}

unsigned long stripRinger(byte mode, unsigned long lampRefer, float stripDuration, float baseTimeSpan, unsigned long ringCount, byte reason) {
  float rollingInterval;

  if (mode == LINEAR_STRIP) {
    rollingInterval = baseTimeSpan + stripDuration*ringCount;
  } else if (mode == GEOMETRIC_STRIP) {
    rollingInterval = baseTimeSpan*pow(2,(1.*ringCount)/(1.*reason));
  } else {
    return -1;
  }

  if (millis()>= lampRefer - 250 + rollingInterval ) {
    tone(buzPin, 659, 150);
  }

  if (millis() >= lampRefer + rollingInterval) {
    tone(buzPin, 880, 100);
    ringCount++;
  }
  return ringCount;
}

unsigned long metronome(unsigned long lampRefer, unsigned long ringCount) {
  if (millis()>=lampRefer+ringCount*1000) { // buzz every second
    tone(buzPin, 987, 100);
    ringCount++;
  }
  return ringCount;
}

byte updateRunningMode(byte runningMode, bool linGeo, bool sinStrip) {
  byte newRunningMode;
  if (!linGeo) {
    if (!sinStrip) {
      newRunningMode = LINEAR_SINGLE;
    } else {
      newRunningMode = LINEAR_STRIP;
    }
  } else {
    if (!sinStrip) {
      newRunningMode = GEOMETRIC_SINGLE;
    } else {
      newRunningMode = GEOMETRIC_STRIP;
    }
  }
  if (newRunningMode != runningMode) {
    lcd.clear();
  }
  return newRunningMode;
}

byte updateRunningStatus(byte runningStatus, bool & runningStatusChanged, bool startRose, bool focusHigh, unsigned long rollingTime) {
  byte newRunningStatus = runningStatus;
  switch (runningStatus) {
    case SETUP:
      if (startRose) {
        newRunningStatus = EXPOSURE;
      } else if (focusHigh) {
        newRunningStatus = FOCUS;
      } 
      break;
    case EXPOSURE:
      if (startRose || (rollingTime>0 && rollingTime<10)) {
        newRunningStatus = SETUP;
      } 
      break;
    case FOCUS:
      if (!focusHigh) {
        newRunningStatus = SETUP;
      } 
      break;
  }
  if (runningStatus != newRunningStatus) {
    runningStatusChanged = 1;
    runningStatus = newRunningStatus;
  } else {
    runningStatusChanged = 0;
  }
  return newRunningStatus;
}

unsigned long updateEvalTimeSpan(byte mode, unsigned long base, int index, byte reason, unsigned long duration, byte strip){
  unsigned long eval;
  switch (mode) {
      case LINEAR_SINGLE:
      eval = base;
      break;
      case GEOMETRIC_SINGLE:
      eval = base*pow(2,(1.*index)/(1.*reason));
      break;
      case LINEAR_STRIP:
      eval = base+(duration*strip);
      break;
      case GEOMETRIC_STRIP:
      eval = base*pow(2,(1.*strip)/(1.*reason));
      break;
    }  
  return eval; 
}

unsigned long updateBaseTimeSpan(unsigned long baseTimeSpan) {   
  for (int i = 0; i < 4; i++) {
    if (pressHoldButton(i)) {
      baseTimeSpan = changeSpanLinearly(i,baseTimeSpan);
    }
  }
  return baseTimeSpan;
}
