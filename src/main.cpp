// Copyright (c) 2022 Inaba
// This software is released under the MIT License.
// http://opensource.org/licenses/mit-license.php

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <Ambient.h>
#include <SparkFun_SCD30_Arduino_Library.h>
#include "settings.h"

SCD30 airSensor;
WiFiClient client;
Ambient* ambient = nullptr;

RTC_DATA_ATTR int wakeupCounter = 0;

Settings& Storage() {
  return Settings::Instance();
}

bool beginWiFi(const char* ssid, const char* psk) {
  const auto start = ::millis();
  WiFi.begin(ssid, psk);
  while (WiFi.status() != WL_CONNECTED) {
    if ((::millis() - start) > 10000) {
      ESP_LOGE("", "Failed to begin WiFi.");
      return false;
    }
    ::delay(100);
  }
  return true;
}

bool loadSettings() {
  Storage().load();

  if (Storage().MeasureCycle() <= 0 || Storage().PostCycle() <= 0) {
    return false;
  }
  wakeupCounter %= Storage().PostCycle() / Storage().MeasureCycle();

  if (wakeupCounter == 0 &&
      Storage().AP_SSID().length() > 0 && Storage().AP_PSK().length() > 0 &&
      Storage().AmbientID() >= 0 && Storage().AmbientKey().length() > 0) {
    if (::beginWiFi(Storage().AP_SSID().c_str(), Storage().AP_PSK().c_str())) {
      ambient = new Ambient();
      ambient->begin(
        Storage().AmbientID(), Storage().AmbientKey().c_str(), &client);
    }
  }
  return true;
}

uint32_t Color(int ppm) {
  if (ppm <= 1000)
    return Storage().Color_Good();
  else if (ppm <= 1500)
    return Storage().Color_Better();
  else if (ppm <= 2500)
    return Storage().Color_Bad();

  return Storage().Color_TooBad();
}

void setup() {
  Serial.begin(115200);
  Storage().begin();

  Wire.begin(1, 0);

  if (!loadSettings()) {
    ESP_LOGE("", "Failed to load configuration file.");
  }
  if (airSensor.begin() == false) {
    ESP_LOGE("", "Failed to begin SCD30.");
    ::esp_deep_sleep(0);
    for (;;) {}
  }
}

void loop() {
  if (!airSensor.dataAvailable()) {
    return;
  }

  const auto co2 = airSensor.getCO2();
  const auto temper = airSensor.getTemperature();
  const auto humidity = airSensor.getHumidity();

  if (co2 <= 0 || temper <= 0.f || humidity <= 0.f) {
    return;
  }

  auto led = new Adafruit_NeoPixel(
    Storage().LedCount(), Storage().LedPin(), NEO_GRB + NEO_KHZ800);
  led->begin();
  const auto color = Color(co2);
  for (auto i = 0; i < led->numPixels(); ++i) {
    led->setPixelColor(i, color);
  }
  led->show();

  Serial.printf("CO2: %d ppm, ", co2);
  Serial.printf("Temperature: %f C, ", temper);
  Serial.printf("Humidity: %f %\r\n", humidity);

  if (ambient) {
    ambient->set(1, String(co2).c_str());
    ambient->set(2, String(temper).c_str());
    ambient->set(3, String(humidity).c_str());
    ambient->send();

    Wire.end();
    WiFi.disconnect();
  }
  wakeupCounter++;

  Serial.println("Zzz...");
  auto interval = Storage().MeasureCycle() * 1000000;
  interval -= ::millis() * 1000;
  ::esp_deep_sleep(interval);
  for (;;) {}
  // never reach
}
