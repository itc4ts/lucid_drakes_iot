#if !defined(LUCID_DRAKE_WEBSERVER_COMMON_H)
#define LUCID_DRAKE_WEBSERVER_COMMON_H

#include <Arduino.h>
#include <FastLED.h>

#include <LittleFS.h>
#define MYFS LittleFS

#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <WiFiUdp.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>

#include "include/simplehacks/static_eval.h"
#include "include/simplehacks/constexpr_strlen.h"
#include "include/simplehacks/array_size2.h"

#include "include/FSBrowser.hpp"


extern WiFiManager wifiManager;
extern ESP8266WebServer webServer;
extern String nameString;

// Structures
typedef void (*Pattern)();
typedef struct {
  Pattern pattern;
  String name;
} PatternAndName;

typedef struct {
  CRGBPalette16 palette;
  String name;
} PaletteAndName;


void sendString(String value);
void sendInt(uint8_t value);
void setPattern(uint8_t value);
void adjustPattern(bool up);

#endif // LUCID_DRAKE_WEBSERVER_COMMON_H