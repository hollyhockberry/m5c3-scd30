// Copyright (c) 2022 Inaba
// This software is released under the MIT License.
// http://opensource.org/licenses/mit-license.php

#ifndef CONTROLPANEL_H_
#define CONTROLPANEL_H_

#include <Adafruit_NeoPixel.h>
#include "settings.h"

class ControlPanel {
 public:
  static bool begin(Settings& settings, Adafruit_NeoPixel* led);
};

#endif  // CONTROLPANEL_H_

