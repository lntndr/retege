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
#include "retege.h"

// runningMode possible values
#define LINEAR_EXPOSURE 0
#define GEOMETRIC_EXPOSURE 1
#define LINEAR_TEST 2
#define GEOMETRIC_TEST 3

// runningStatus possible values
#define SETUP 0
#define EXPOSURE 1
#define FOCUS 2

// buttons coding
#define S_BUTTON 0
#define N_BUTTON 1
#define W_BUTTON 2
#define E_BUTTON 3                           
#define START_BUTTON 4
#define FOCUS_TOGGLE 5
#define LINEAR_GEOMETRIC_TOGGLE 6
#define EXPOSURE_TEST_TOGGLE 7

// pinout
#define RELAY 13
#define BUZZER 19

// objects
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);
Bounce * input = new Bounce[8];
retege mTimer = retege();

// function declarations
bool buttonActive(int i);
char updateRunningStatus(char runningStatus, bool & runningStatusChanged,
  bool startRose, bool focusHigh, unsigned long rollingTime);
char updateRunningMode(bool linGeo, bool sinStrip);

/// buzzer functions
unsigned long playEndStrip(char mode, unsigned long lampRefer,
  unsigned long ringCount);
unsigned long playMetronome(unsigned long lampRefer, unsigned long ringCount);

// arduino functions

void setup() {
  // local variables
  /// buttons
  const int sPin = 2, nPin = 3, startPin = 4, wPin = 5, ePin = 6;

  /// toggles
  const int focusPin = 14, linGeoPin = 15, expTestPin = 16;

  /// bounce2 aux array
  const int inputPins[8] = {sPin, nPin,
                            wPin, ePin,
                            startPin, focusPin,
                            linGeoPin, expTestPin};

  // declarations
  pinMode(RELAY,  OUTPUT);
  pinMode(BUZZER, OUTPUT);

  // run things
  for (int i = 0; i < 8; i++) {
    input[i].attach(inputPins[i], INPUT);
    input[i].interval(25);
  }

  lcd.begin(16,2);
  lcd.print("retege v0.3");
  delay(2000);
  lcd.clear();
}

void loop() {
  // local variables
  // 0 = linear single, 1 = geometric single;
  // 2 = linear strip, 3 = geometric strip
  static char runningMode = LINEAR_EXPOSURE;
  static char newRunningMode = LINEAR_EXPOSURE;
  // 0 = setup, 1 = exposure, 2 = focus
  static char runningStatus = SETUP;
  static bool runningStatusChanged = 1;

  // sound and countup/down
  static unsigned long lampFirstMillis = 0;
  static unsigned long ringCount = 1;
  static unsigned long prevRollingTime = 0;
  static unsigned long rollingTime = 0;

  // update buttons
  for (int i = 0; i < 8; i++) {
    input[i].update();
    if (input[i].fell() || input[i].rose()) {
      lcd.clear();
    }
  }

  runningStatus = updateRunningStatus(runningStatus, runningStatusChanged,
    input[START_BUTTON].rose(), input[FOCUS_TOGGLE].read(), rollingTime);

  switch (runningStatus) {
    case SETUP:
      // turn off the lamp
      digitalWrite(RELAY,LOW);

      // runningMode independent
      if (runningStatusChanged) {
        ringCount = 1;
        rollingTime = 0;
        lampFirstMillis = 0;
        lcd.clear();
      }

      // runningMode dependent
      newRunningMode = updateRunningMode(
        input[LINEAR_GEOMETRIC_TOGGLE].read(),
        input[EXPOSURE_TEST_TOGGLE].read());

      if (newRunningMode != runningMode) {
        runningMode = newRunningMode;
        lcd.clear();
      }

      switch (runningMode) {
        case LINEAR_EXPOSURE:
          mTimer.updateBaseTimeSeconds(buttonActive(N_BUTTON),
            buttonActive(S_BUTTON));
          mTimer.updateBaseTimeTenths(buttonActive(E_BUTTON),
            buttonActive(W_BUTTON));
          break;
        case GEOMETRIC_EXPOSURE:
          mTimer.updateReason(buttonActive(N_BUTTON),
            buttonActive(S_BUTTON));
          mTimer.updateIndex(buttonActive(E_BUTTON),
            buttonActive(W_BUTTON));
          break;
        case LINEAR_TEST:
          mTimer.updateStripDuration(buttonActive(N_BUTTON),
            buttonActive(S_BUTTON));
          mTimer.updateStripNumber(buttonActive(E_BUTTON),
            buttonActive(W_BUTTON));
          break;
        case GEOMETRIC_TEST:
          mTimer.updateReason(buttonActive(N_BUTTON),
            buttonActive(S_BUTTON));
          mTimer.updateStripNumber(buttonActive(E_BUTTON),
            buttonActive(W_BUTTON));
          break;
      }
      break;

    case EXPOSURE:
      // runningMode independent
      digitalWrite(RELAY,HIGH);

      if (runningStatusChanged) {
        rollingTime=mTimer.evaluateTime(runningMode);
        lampFirstMillis = millis();
        lcd.clear();
      }

      prevRollingTime = rollingTime;
      rollingTime = (lampFirstMillis+(mTimer.evaluateTime(runningMode))
        -millis());
      if (rollingTime>prevRollingTime) {
        rollingTime=0;
      }

      // runningMode dependent
      switch(runningMode) {
        case LINEAR_EXPOSURE:
        case GEOMETRIC_EXPOSURE:
          ringCount = playMetronome(lampFirstMillis,ringCount);
          break;
        case LINEAR_TEST:
        case GEOMETRIC_TEST:
          ringCount = playEndStrip(runningMode, lampFirstMillis, ringCount);
          break;
      }

      break;

    case FOCUS:
      // mode independent
      if (runningStatusChanged) {
        lampFirstMillis = millis();
        lcd.clear();
      }
      digitalWrite(RELAY,HIGH);
      ringCount = playMetronome(lampFirstMillis,ringCount);
      break;
  }
  // loop() END
}

// service functions

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
  for (char k = 1; k < 5; k++) {
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

char updateRunningStatus(char runningStatus, bool & runningStatusChanged,
  bool startRose, bool focusHigh, unsigned long rollingTime) {
  char newRunningStatus = runningStatus;
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

char updateRunningMode(bool linGeo, bool expTest) {
  char newRunningMode;
  if (!linGeo) {
    if (!expTest) {
      newRunningMode = LINEAR_EXPOSURE;
    } else {
      newRunningMode = LINEAR_TEST;
    }
  } else {
    if (!expTest) {
      newRunningMode = GEOMETRIC_EXPOSURE;
    } else {
      newRunningMode = GEOMETRIC_TEST;
    }
  }
  return newRunningMode;
}

// beeper function

unsigned long playEndStrip(char mode, unsigned long lampRefer,
  unsigned long ringCount) {

  float rollingInterval;

  if (mode == LINEAR_TEST) {
    rollingInterval = mTimer.getBaseTime() + mTimer.getstripDrt()*ringCount;
  } else if (mode == GEOMETRIC_TEST) {
    rollingInterval = mTimer.getBaseTime() * 
      pow(2,(1.*ringCount)/(1.*mTimer.getReason()));
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