// Copyright (c) 2022 Inaba
// This software is released under the MIT License.
// http://opensource.org/licenses/mit-license.php

#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <SPIFFS.h>

#include "controlpanel.h"

namespace {

Settings* settings_ = nullptr;
AsyncWebServer server(80);

String colorString(uint32_t col) {
  auto s = String("000000") + String(col, HEX);
  return s.substring(s.length() - 6);
}

String templateProcessor(const String& var) {
  if (var == "SSID") {
    return settings_->AP_SSID();
  } else if (var == "MEAS_CYC") {
    return String(settings_->MeasureCycle());
  } else if (var == "POST_CYC") {
    return String(settings_->PostCycle());
  } else if (var == "AMB_ID") {
    return String(settings_->AmbientID());
  } else if (var == "AMB_KEY") {
    return settings_->AmbientKey();
  } else if (var == "PIN_LED") {
    return String(settings_->LedPin());
  } else if (var == "NUM_LED") {
    return String(settings_->LedCount());
  } else if (var == "COLOR_GOOD") {
    return colorString(settings_->Color_Good());
  } else if (var == "COLOR_BETTER") {
    return colorString(settings_->Color_Better());
  } else if (var == "COLOR_BAD") {
    return colorString(settings_->Color_Bad());
  } else if (var == "COLOR_2BAD") {
    return colorString(settings_->Color_TooBad());
  }
  return String();
}

}  // namespace

bool ControlPanel::begin(Settings& settings, Adafruit_NeoPixel* led) {
  if (!WiFi.mode(WIFI_AP)) {
    return false;
  }
  if (!WiFi.softAP("AP_Sens")) {
    return false;
  }
  if (!MDNS.begin("sensedge")) {
    return false;
  }
  settings_ = &settings;

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/index.html", String(), false, templateProcessor);
  });
  server.on("/setwifi", HTTP_ANY, [](AsyncWebServerRequest *request) {
    settings_->save(request);
    request->send(200);
  });
  server.on("/set", HTTP_ANY, [](AsyncWebServerRequest *request) {
    settings_->save(request);
    request->send(200);
  });

  server.begin();

  if (led) {
    led->begin();
    for (auto i = 0; i < led->numPixels(); ++i) {
      led->setPixelColor(i, 0xff00ff);
    }
    led->show();
  }
  return true;
}

