/*  Copyright 2020 Andrea Lanterna
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
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

#define W_BUTTON 0
#define S_BUTTON 1
#define E_BUTTON 2
#define N_BUTTON 3                           
#define START_BUTTON 4
#define FOCUS_TOGGLE 5
#define LINEAR_GEOMETRIC_TOGGLE 6
#define SINGLE_STRIP_TOGGLE 7

#define RELAY 13
#define BUZZER 19

// objects
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);
Bounce * input = new Bounce[8];

// functions declarations

/// service function
bool buttonActive(int i);
float changeSpanLinearly(int j, float base);

/// buzzer functions
unsigned long playEndStrip(byte mode, unsigned long lampRefer, float stripDuration, float baseTimeSpan, unsigned long ringCount, byte reason);
unsigned long playMetronome(unsigned long lampRefer, unsigned long ringCount);

/// state variables functions
byte updateRunningMode(byte runningMode, bool linGeo, bool sinStrip);
byte updateRunningStatus(byte runningStatus, bool & runningStatusChanged, bool startRose, bool focusHigh, unsigned long rollingTime);

/// possible timer class?
unsigned long updateEvalTimeSpan(byte mode, unsigned long base, int index, byte reason, unsigned long duration, byte strip);
unsigned long updateBaseTimeSpan(unsigned long baseTimeSpan);
byte updateGeometricReason(byte reason);
int updateGeometricIndex(int index);
byte updateStripNumber(byte strip);
unsigned long updateStripDuration(unsigned long duration);

void setup() {
  // local variables
  /// buttons
  const int wPin = 2, sPin = 3, startPin = 4, ePin = 5, nPin = 6;

  /// toggles
  const int focusPin = 14, linGeoPin = 15, singStriPin = 16;

  /// bounce2 aux array
  const int inputPins[8] = {wPin, sPin,
                            ePin, nPin,
                            startPin,
                            focusPin, linGeoPin, singStriPin};

  // declarations
  pinMode(RELAY,  OUTPUT);
  pinMode(BUZZER, OUTPUT);

  pinMode(ePin,    INPUT);
  pinMode(nPin,    INPUT);
  pinMode(startPin, INPUT);
  pinMode(wPin,    INPUT);
  pinMode(sPin,    INPUT);

  pinMode(focusPin,    INPUT);
  pinMode(linGeoPin,   INPUT);
  pinMode(singStriPin, INPUT);

  // run things
  for (int i = 0; i < 8; i++) {
    input[i].attach(inputPins[i], INPUT);
    input[i].interval(40);
  }

  lcd.begin(16,2);
  lcd.print("retege v0.2");
  delay(2000);
  lcd.clear();

  Serial.begin(9600);
}

void loop() {
  // local variables
  // 0 = linear single, 1 = geometric single, 2 = linear strip, 3 = geometric strip
  static byte runningMode = LINEAR_SINGLE;
  // 0 = setup, 1 = exposure, 2 = focus
  static byte runningStatus = SETUP;
  static bool runningStatusChanged = 1;

  // possible timer class?
  static unsigned long baseTimeSpan = 5000;
  static unsigned long evalTimeSpan = 0;
  static byte stripNumber = 10;
  static unsigned long stripDuration = 2000;
  static int geometricIndex = 0;
  static byte geometricReason = 3;

  // sound and countup/down
  static unsigned long lampFirstMillis = 0;
  static unsigned long ringCount = 1;
  static unsigned long rollingTime = 0;

  // update buttons
  for (int i = 0; i < 8; i++) {
    input[i].update();
    if (input[i].fell() || input[i].rose()) {
      lcd.clear();
    }
  }
  runningStatus = updateRunningStatus(runningStatus, runningStatusChanged, input[START_BUTTON].rose(), input[FOCUS_TOGGLE].read(), rollingTime);
  switch (runningStatus) {
    case SETUP:
      // mode independent
      if (runningStatusChanged) {
        ringCount = 1;
        rollingTime = 0;
        lampFirstMillis = 0;
        lcd.clear();
      }
      // turn off the lamp
      digitalWrite(RELAY,LOW);
      runningMode = updateRunningMode(runningMode,input[LINEAR_GEOMETRIC_TOGGLE].read(),input[SINGLE_STRIP_TOGGLE].read());
      // update evalTimeSpangi
      evalTimeSpan = updateEvalTimeSpan(runningMode, baseTimeSpan, geometricIndex, geometricReason, stripDuration, stripNumber);
      // read buttons
      lcd.setCursor(0,0);
      lcd.print("SETUP");
      lcd.setCursor(0,1);

      // mode dependent
      switch (runningMode) {
        case LINEAR_SINGLE:
          baseTimeSpan = updateBaseTimeSpan(baseTimeSpan);
          lcd.print((float)evalTimeSpan/(float)1000,1);
          break;

        case GEOMETRIC_SINGLE:
          geometricReason = updateGeometricReason(geometricReason);
          geometricIndex = updateGeometricIndex(geometricIndex);

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
          stripDuration = updateStripDuration(stripDuration);
          stripNumber = updateStripNumber(stripNumber);

          lcd.print(baseTimeSpan/1000.,1);
          lcd.print("+[");
          lcd.print(stripNumber);
          lcd.print("*");
          lcd.print(stripDuration/1000.,1);
          lcd.print("s]");
          break;

        case GEOMETRIC_STRIP:
          geometricReason = updateGeometricReason(geometricReason);
          stripNumber = updateStripNumber(stripNumber);

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
      // mode independent
      if (runningStatusChanged) {
        rollingTime=evalTimeSpan;
        lampFirstMillis = millis() + 10; // 10 ms buffer allows to use unsigned long
        lcd.clear();
      }
      digitalWrite(RELAY,HIGH);
      rollingTime = (lampFirstMillis+(evalTimeSpan)-millis());

      lcd.setCursor(0,0);
      lcd.print("EXPOSURE");
      lcd.setCursor(0,1);
      lcd.print(rollingTime/1000.,1);
      if ((int)log10(rollingTime)+1 != (int)log10(rollingTime+100)+1) {
          //clear the screen when the number of digit changes
          lcd.clear(); 
      }

      // mode dependent
      switch(runningMode) {
        case LINEAR_SINGLE:
        case GEOMETRIC_SINGLE:
          ringCount = playMetronome(lampFirstMillis,ringCount);
          break;
        case LINEAR_STRIP:
        case GEOMETRIC_STRIP:
          lcd.print("  ");
          lcd.print(ringCount);
          lcd.print("/");
          lcd.print(stripNumber);
          ringCount = playEndStrip(runningMode, lampFirstMillis, stripDuration, baseTimeSpan, ringCount, geometricReason);
          break;
      }
      
      break; // end EXPOSURE case

    case FOCUS:
      // mode independent
      if (runningStatusChanged) {
        lampFirstMillis = millis() + 10; // 10 ms buffer allows to use unsigned long
        lcd.clear();
      }
      digitalWrite(RELAY,HIGH);
      ringCount = playMetronome(lampFirstMillis,ringCount);
      lcd.setCursor(0,0);
      lcd.print("FOCUS");
      lcd.setCursor(0,1);
      lcd.print((millis()-lampFirstMillis+10)/1000.,1);
      break;
  }
  // loop() END
}

bool buttonActive(int i) {
  static unsigned long holdInterval = 0;
  static int holdMillis = 100;
  bool y;

  if(input[i].read() && input[i].duration()>=holdInterval*(holdMillis)) {
    y = 1;
    holdInterval++;
  } else {
    y = 0;
  }

  // it goes progressively faster when long pressed
  for (byte k = 1; k < 5; k++) {
    if (input[i].read() && input[i].duration()>k*1000) {
      holdMillis = 100-(k*20);
    }
  }

  if (input[i].fell()) {
    holdInterval = 0;
    holdMillis = 100;
  }
  return y;
}

unsigned long playEndStrip(byte mode, unsigned long lampRefer, float stripDuration, float baseTimeSpan, unsigned long ringCount, byte reason) {
  float rollingInterval;

  if (mode == LINEAR_STRIP) {
    rollingInterval = baseTimeSpan + stripDuration*ringCount;
  } else if (mode == GEOMETRIC_STRIP) {
    rollingInterval = baseTimeSpan*pow(2,(1.*ringCount)/(1.*reason));
  } else {
    return -1;
  }

  if (millis()>= lampRefer - 100 + rollingInterval ) {
    tone(BUZZER, 659, 100);
  }

  if (millis() >= lampRefer + rollingInterval) {
    tone(BUZZER, 880, 100);
    ringCount++;
  }
  return ringCount;
}

unsigned long playMetronome(unsigned long lampRefer, unsigned long ringCount) {
  if (millis()>=lampRefer+ringCount*1000) { // buzz every second
    tone(BUZZER, 987, 100);
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
      eval = base*(pow((float)2,((float)index)/((float)reason)));
      break;
      case LINEAR_STRIP:
      eval = base+(duration*strip);
      break;
      case GEOMETRIC_STRIP:
      eval = base*(pow((float)2,((float)strip)/((float)reason)));
      break;
    }  
  return eval; 
}

unsigned long updateBaseTimeSpan(unsigned long base) {
  unsigned long newb = base;
  int k;
  if (buttonActive(E_BUTTON)) {
    k = 100;
  } else if (buttonActive(W_BUTTON)) {
    k = -100;
  } else if (buttonActive(N_BUTTON)) {
    k = 1000;
  } else if (buttonActive(S_BUTTON)) {
    k = -1000;
  } else {
    k = 0;
  }
  if ((long)base+k < 0) {
    newb = 0;
  } else if (base+k > 999900) {
    newb = 999900;
  } else {
    newb = base+k;
  }
  if ((int)log10(newb)+1 > (int)log10(base)+1) {
    lcd.clear(); 
  }
  return newb;

}

byte updateGeometricReason(byte reason) {
  if (buttonActive(N_BUTTON)) {
    reason++;
  } else if (buttonActive(S_BUTTON) && reason > 1) {
    reason--;
  }
  return reason;
}

int updateGeometricIndex(int index) {
  if (buttonActive(E_BUTTON)) {
    index++;
  } else if (buttonActive(W_BUTTON)) {
    index--;
  }
  return index;
}

byte updateStripNumber(byte strip) {
  if (buttonActive(E_BUTTON)) {
    strip++;
  } else if (buttonActive(W_BUTTON) && strip > 1) {
    strip--;
  }
  return strip;
}

unsigned long updateStripDuration(unsigned long duration) {
  if (buttonActive(N_BUTTON)) {
    duration = duration+100;
  } else if (buttonActive(S_BUTTON) && duration > 0) {
    duration = duration-100;
  }
  return duration;
}