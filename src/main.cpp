// Copyright (c) 2022 Inaba
// This software is released under the MIT License.
// http://opensource.org/licenses/mit-license.php

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "settings.h"
#include "sensor.h"
#include "controlpanel.h"

Settings settings;
Adafruit_NeoPixel* led = nullptr;
bool serviceMode = false;

void setup() {
  Serial.begin(115200);
  settings.begin();
  settings.load();

  led = new Adafruit_NeoPixel(
    settings.LedCount(), settings.LedPin(), NEO_GRB + NEO_KHZ800);

  ::pinMode(3, INPUT_PULLUP);
  serviceMode = (::digitalRead(3) == LOW)
    ? ControlPanel::begin(settings, led) : false;

  if (!serviceMode) {
    Sensor::begin(settings);
  }
}

void loop() {
  if (!serviceMode) {
    Sensor::loop(settings, led);
  }
}
