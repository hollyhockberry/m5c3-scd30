// Copyright (c) 2021 Inaba
// This software is released under the MIT License.
// http://opensource.org/licenses/mit-license.php

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <ArduinoJson.h>
#include <Ambient.h>
#include <FS.h>
#include <SparkFun_SCD30_Arduino_Library.h>
#include <SPIFFS.h>

int PERIOD_SEC = 5 * 60;
int PIN_LED = 2;
int NUM_LED = 1;

SCD30 airSensor;
WiFiClient client;
Ambient* ambient = nullptr;

bool loadSettings() {
  auto file = SPIFFS.open("/settings.json", "r");
  if (!file || file.size() == 0) {
    return false;
  }

  DynamicJsonDocument json(1300);
  auto err = ::deserializeJson(json, file);
  if (err) {
    return false;
  }

  if (json.containsKey("Inverval")) {
    PERIOD_SEC = static_cast<int>(json["Inverval"]);
  }
  if (json.containsKey("LED_Pin")) {
    PIN_LED = static_cast<int>(json["LED_Pin"]);
  }
  if (json.containsKey("LED_Num")) {
    NUM_LED = static_cast<int>(json["LED_Num"]);
  }

  if (json.containsKey("SSID") && json.containsKey("PSK") &&
      json.containsKey("Amb_ID") && json.containsKey("Amb_KEY")) {
    const char* SSID = json["SSID"];
    const char* PSK = json["PSK"];
    const int ID = json["Amb_ID"];
    const char* writeKey = json["Amb_KEY"];

    WiFi.begin(SSID, PSK);
    while (WiFi.status() != WL_CONNECTED) {
      ::delay(100);
    }
    ambient = new Ambient();
    ambient->begin(ID, writeKey, &client);
  }
  return true;
}

uint32_t Color(int ppm) {
  if (ppm <= 1000)
    return Adafruit_NeoPixel::Color(0, 128, 0);  // good: green
  else if (ppm <= 1500)
    return Adafruit_NeoPixel::Color(255, 255, 0);  // slightly better: yellow
  else if (ppm <= 2500)
    return Adafruit_NeoPixel::Color(255, 65, 0);  // bad: orange

  return Adafruit_NeoPixel::Color(255, 0, 0);  // very bad: red
}

void setup() {
  Serial.begin(115200);
  SPIFFS.begin(true);
  Wire.begin(1, 0);

  if (!loadSettings()) {
    ESP_LOGE("", "Failed to load configuration file.");
    for (;;) {}
  }
  if (airSensor.begin() == false) {
    ESP_LOGE("", "Failed to begin SCD30.");
    ::esp_restart();
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

  Adafruit_NeoPixel led(NUM_LED, PIN_LED, NEO_GRB + NEO_KHZ800);
  led.begin();
  const auto color = Color(co2);
  for (auto i = 0; i < led.numPixels(); ++i) {
    led.setPixelColor(i, color);
  }
  led.show();

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

    delete ambient;
    ambient = nullptr;
  }

  Serial.println("Zzz...");
  auto interval = PERIOD_SEC * 1000000;
  interval -= ::millis() * 1000;
  ::esp_deep_sleep(interval);
  for (;;) {}
  // never reach
}
