// Copyright (c) 2022 Inaba
// This software is released under the MIT License.
// http://opensource.org/licenses/mit-license.php

#include <Arduino.h>
#include "settings.h"
#include "sensor.h"

Settings settings;

void setup() {
  Serial.begin(115200);
  settings.begin();
  settings.load();

  Sensor::begin(settings);
}

void loop() {
  Sensor::loop(settings);
}
