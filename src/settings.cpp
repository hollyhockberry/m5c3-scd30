// Copyright (c) 2022 Inaba
// This software is released under the MIT License.
// http://opensource.org/licenses/mit-license.php

#include "settings.h"
#include <ArduinoJson.h>
#include <SPIFFS.h>

namespace {

int PERIOD_SEC = 5 * 60;
int POST_PERIOD_SEC = PERIOD_SEC;
int PIN_LED = 2;
int NUM_LED = 1;

uint32_t COLOR_GOOD = 0x008000;
uint32_t COLOR_BETTER = 0xffff00;
uint32_t COLOR_BAD = 0xff4100;
uint32_t COLOR_TOOBAD = 0xff0000;

String SSID("");
String PSK("");
int AmbID = -1;
String AmbKey("");

template<typename T>
T Key(const char* key, const T initial, const DynamicJsonDocument& json) {
  return json.containsKey(key) ? static_cast<T>(json[key]) : initial;
}

int color2int(const String str) {
  return ::strtol(str.substring(1).c_str(), nullptr, 16);
}

}  // namespace

Settings& Settings::Instance() {
  static Settings o;
  return o;
}

void Settings::begin() {
  SPIFFS.begin(true);
}

void Settings::load() {
  auto file = SPIFFS.open("/settings.json", "r");
  if (!file || file.size() == 0) {
    return;
  }
  DynamicJsonDocument json(1300);
  auto err = ::deserializeJson(json, file);
  if (err) {
    return;
  }
  PERIOD_SEC = Key<int>("Interval", PERIOD_SEC, json);
  POST_PERIOD_SEC = Key<int>("PostInterval", POST_PERIOD_SEC, json);
  if (POST_PERIOD_SEC < PERIOD_SEC) {
    POST_PERIOD_SEC = PERIOD_SEC;
  }

  PIN_LED = Key<int>("LED_Pin", PIN_LED, json);
  NUM_LED = Key<int>("LED_Num", NUM_LED, json);
  COLOR_GOOD = Key<uint32_t>("LED_Good", COLOR_GOOD, json);
  COLOR_BETTER = Key<uint32_t>("LED_Better", COLOR_BETTER, json);
  COLOR_BAD = Key<uint32_t>("LED_Bad", COLOR_BAD, json);
  COLOR_TOOBAD = Key<uint32_t>("LED_TooBad", COLOR_TOOBAD, json);

  SSID = Key<const char*>("SSID", SSID.c_str(), json);
  PSK = Key<const char*>("PSK", PSK.c_str(), json);
  AmbID = Key<int>("Amb_ID", AmbID, json);
  AmbKey = Key<const char*>("Amb_KEY", AmbKey.c_str(), json);
}

int Settings::MeasureCycle() const {
  return PERIOD_SEC;
}

int Settings::PostCycle() const {
  return POST_PERIOD_SEC;
}

int Settings::LedPin() const {
  return PIN_LED;
}

int Settings::LedCount() const {
  return NUM_LED;
}

uint32_t Settings::Color_Good() const {
  return COLOR_GOOD;
}

uint32_t Settings::Color_Better() const {
  return COLOR_BETTER;
}

uint32_t Settings::Color_Bad() const {
  return COLOR_BAD;
}

uint32_t Settings::Color_TooBad() const {
  return COLOR_TOOBAD;
}

const String& Settings::AP_SSID() const {
  return SSID;
}

const String& Settings::AP_PSK() const {
  return PSK;
}

int Settings::AmbientID() const {
  return AmbID;
}

const String& Settings::AmbientKey() const {
  return AmbKey;
}
