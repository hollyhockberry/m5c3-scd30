// Copyright (c) 2022 Inaba
// This software is released under the MIT License.
// http://opensource.org/licenses/mit-license.php

#include <Adafruit_NeoPixel.h>
#include <Ambient.h>
#include <SparkFun_SCD30_Arduino_Library.h>
#include "sensor.h"

namespace {

RTC_DATA_ATTR int wakeupCounter = 0;

SCD30 airSensor;
WiFiClient wifiClient;
Ambient* ambient;

bool beginWiFi(const Settings& settings) {
  const auto start = ::millis();
  WiFi.begin(settings.AP_SSID().c_str(), settings.AP_PSK().c_str());
  while (WiFi.status() != WL_CONNECTED) {
    if ((::millis() - start) > 10000) {
      ESP_LOGE("", "Failed to begin WiFi.");
      return false;
    }
    ::delay(100);
  }
  return true;
}

uint32_t Color(int ppm, const Settings& settings) {
  if (ppm <= 1000)
    return settings.Color_Good();
  else if (ppm <= 1500)
    return settings.Color_Better();
  else if (ppm <= 2500)
    return settings.Color_Bad();

  return settings.Color_TooBad();
}

}  // namespace

void Sensor::begin(const Settings& settings) {
  Wire.begin(1, 0);

  if (settings.MeasureCycle() <= 0 || settings.PostCycle() <= 0) {
    return;
  }

  if (airSensor.begin() == false) {
    ESP_LOGE("", "Failed to begin SCD30.");
    ::esp_deep_sleep(0);
    for (;;) {}
    // never reach
  }

  wakeupCounter %= settings.PostCycle() / settings.MeasureCycle();

  if (wakeupCounter == 0 &&
      settings.AP_SSID().length() > 0 && settings.AP_PSK().length() > 0 &&
      settings.AmbientID() >= 0 && settings.AmbientKey().length() > 0) {
    if (::beginWiFi(settings)) {
      ambient = new Ambient();
      ambient->begin(
        settings.AmbientID(), settings.AmbientKey().c_str(), &wifiClient);
    }
  }
}

void Sensor::loop(const Settings& settings) {
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
    settings.LedCount(), settings.LedPin(), NEO_GRB + NEO_KHZ800);

  led->begin();
  const auto color = Color(co2, settings);
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
  auto interval = settings.MeasureCycle() * 1000000;
  interval -= ::millis() * 1000;
  ::esp_deep_sleep(interval);
  for (;;) {}
  // never reach
}
