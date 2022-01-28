// Copyright (c) 2022 Inaba
// This software is released under the MIT License.
// http://opensource.org/licenses/mit-license.php

#ifndef SETTINGS_H_
#define SETTINGS_H_

#include <stdint.h>
#include <WString.h>

class Settings {
 public:
  static Settings& Instance();

  int MeasureCycle() const;
  int PostCycle() const;
  int LedPin() const;
  int LedCount() const;
  uint32_t Color_Good() const;
  uint32_t Color_Better() const;
  uint32_t Color_Bad() const;
  uint32_t Color_TooBad() const;
  const String& AP_SSID() const;
  const String& AP_PSK() const;
  int AmbientID() const;
  const String& AmbientKey() const;

  void begin();
  void load();

 private:
  Settings() {}

  Settings(const Settings&);
  void operator =(const Settings&);
};

#endif  // SETTINGS_H_
