// Copyright (c) 2022 Inaba
// This software is released under the MIT License.
// http://opensource.org/licenses/mit-license.php

#include <Arduino.h>
#include "settings.h"
#include "sensor.h"

void setup() {
  Serial.begin(115200);
  Settings::Instance().begin();
  Settings::Instance().load();

  Sensor::begin(Settings::Instance());
}

void loop() {
  Sensor::loop(Settings::Instance());
}
