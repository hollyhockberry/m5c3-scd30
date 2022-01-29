// Copyright (c) 2022 Inaba
// This software is released under the MIT License.
// http://opensource.org/licenses/mit-license.php

#ifndef SENSOR_H_
#define SENSOR_H_

#include <Adafruit_NeoPixel.h>
#include "settings.h"

class Sensor {
 public:
  static void begin(const Settings& settings);
  static void loop(const Settings& settings, Adafruit_NeoPixel* led);
};

#endif  // SENSOR_H_
