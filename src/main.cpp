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

// objects
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);
Bounce * input = new Bounce[NUM_INPUTS];

void setup() {
  for (int i = 0; i < NUM_INPUTS; i++) {
    input[i].attach(inputPins[i], INPUT);
    input[i].interval(25);
  }
  lcd.begin(16,2);
  lcd.print("main.cpp");
  delay(1000);
  lcd.clear();
}

void loop() {
  // put your main code here, to run repeatedly:
}