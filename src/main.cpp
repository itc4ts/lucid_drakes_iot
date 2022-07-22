

#include "common.h"
#include "config.h"

WiFiManager wifiManager;
ESP8266WebServer webServer(80); 
CRGB leds[NUM_LEDS];


CRGBPalette16 currentPalette;
TBlendType    currentBlending = LINEARBLEND;

uint8_t state = 1;
uint8_t fps = 10;
uint8_t ledIndex = 0;
uint8_t gHue = 0; // rotating "base color" used by many of the patterns

uint8_t currentPatternIndex = 0; // Index du pattern courant

String nameString;


void FillLEDsFromPaletteColors(uint8_t colorIndex)
{
    uint8_t brightness = 255;  
    for( int i = 0; i < NUM_LEDS; i++) {
        leds[i] = ColorFromPalette(currentPalette, colorIndex, brightness, currentBlending);
        colorIndex += 3;
    }
}

void setup() {

  // put your setup code here, to run once:
  Serial.begin(115200);

  // copy the mac address to a byte array
  uint8_t mac[WL_MAC_ADDR_LENGTH];
  WiFi.softAPmacAddress(mac);

  // format the last two digits to hex character array, like 0A0B
  char macID[5];
  sprintf(macID, "%02X%02X", mac[WL_MAC_ADDR_LENGTH - 2], mac[WL_MAC_ADDR_LENGTH - 1]);

  // convert the character array to a string
  String macIdString = macID;
  macIdString.toUpperCase();

  nameString = NAME_PREFIX + macIdString;

  const size_t nameCharCount = static_eval<size_t, constexpr_strlen(NAME_PREFIX) + 4>::value;
  const size_t nameBufferSize = static_eval<size_t, nameCharCount+1>::value;
  char nameChar[nameBufferSize];
  memset(nameChar, 0, nameBufferSize);
  // Technically, this should *NEVER* need to check the nameString length.
  // However, I prefer to code defensively, since no static_assert() can detect this.
  size_t loopUntil = (nameCharCount <= nameString.length() ? nameCharCount : nameString.length());
  for (size_t i = 0; i < loopUntil; i++) {
    nameChar[i] = nameString.charAt(i);
  }

  // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS);

  FastLED.setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);

  fill_solid(currentPalette, NUM_LEDS, CRGB::Red);
  FillLEDsFromPaletteColors(0);
  FastLED.show(); 

  //Forcer à effacer les données WIFI dans l'eprom , permet de changer d'AP à chaque démarrage ou effacer les infos d'une AP dans la mémoire ( à valider , lors du premier lancement  )
  wifiManager.resetSettings();

  Serial.println();
  Serial.println(F("System Info:"));
  Serial.print( F("Heap: ") ); Serial.println(system_get_free_heap_size());
  Serial.print( F("Boot Vers: ") ); Serial.println(system_get_boot_version());
  Serial.print( F("CPU: ") ); Serial.println(system_get_cpu_freq());
  Serial.print( F("SDK: ") ); Serial.println(system_get_sdk_version());
  Serial.print( F("Chip ID: ") ); Serial.println(system_get_chip_id());
  Serial.print( F("Flash ID: ") ); Serial.println(spi_flash_get_id());
  Serial.print( F("Flash Size: ") ); Serial.println(ESP.getFlashChipRealSize());
  Serial.print( F("Vcc: ") ); Serial.println(ESP.getVcc());
  Serial.print( F("MAC Address: ") ); Serial.println(WiFi.macAddress());
  Serial.println();

  if (!MYFS.begin()) {
    Serial.println(F("An error occurred when attempting to mount the flash file system"));
  } else {
    Serial.println("FS contents:");

    Dir dir = MYFS.openDir("/");
    while (dir.next()) {
      String fileName = dir.fileName();
      size_t fileSize = dir.fileSize();
      Serial.printf("FS File: %s, size: %s\n", fileName.c_str(), String(fileSize).c_str());
    }
    Serial.printf("\n");
  }

  wifiManager.setConfigPortalBlocking(false);

  if (wifiManager.autoConnect(nameChar)){
    fill_solid(currentPalette, NUM_LEDS, CRGB::Green);
    Serial.println("Wi-Fi connected");
  }
  else {
    fill_solid(currentPalette, NUM_LEDS, CRGB::Blue);
    Serial.println("Wi-Fi manager portal running");
  }

  MDNS.begin(nameChar);
  MDNS.setHostname(nameChar);


  webServer.on("/pattern", HTTP_POST, []() {
    String value = webServer.arg("value");
    setPattern(value.toInt());
    sendInt(currentPatternIndex);
  });

  webServer.enableCORS(true);
  webServer.serveStatic("/", LittleFS, "/", "max-age=86400");

  webServer.begin();
  Serial.println("HTTP web server started");

  Serial.println("Ready.");
  
  FillLEDsFromPaletteColors(0);
  FastLED.show(); 

}

void sendInt(uint8_t value)
{
  sendString(String(value));
}

void sendString(String value)
{
  webServer.send(200, "text/plain", value);
}

void heartbeat()
{
  uint8_t BeatsPerMinute = 5;
  uint8_t beat = beatsin8( BeatsPerMinute, 8, 192);
  FastLED.setBrightness(beat);
}

void loading()
{
  fadeToBlackBy( leds, NUM_LEDS, 60);
  leds[ledIndex] += CHSV( 0, 255, 192);
  ledIndex += 1;
  if (ledIndex >= NUM_LEDS) { ledIndex = 0; }
}

const PatternAndName patterns[] = {
  { heartbeat,        "Heartbeat" },
  { loading,          "Loading" }
};
const uint8_t patternCount = ARRAY_SIZE2(patterns);

// increase or decrease the current pattern number, and wrap around at the ends
void adjustPattern(bool up)
{
  if (up)
    currentPatternIndex++;
  else
    currentPatternIndex--;

  // wrap around at the end
  if (currentPatternIndex >= patternCount) {
    currentPatternIndex = 0;
  }
}

void setPattern(uint8_t value)
{
  if (value >= patternCount)
    value = patternCount - 1;
  currentPatternIndex = value;
}

void loop() {

  wifiManager.process();
  webServer.handleClient();
  
  patterns[currentPatternIndex].pattern();

  FastLED.delay(1000/fps);     

}

