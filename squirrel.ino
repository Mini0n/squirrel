#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <ESP8266WiFi.h>
#include <TZ.h>
#include <time.h>

// #define TIMEZONE

// -----------------------------------
// OLED Display
U8G2_SSD1306_128X64_NONAME_F_SW_I2C
u8g2(
  U8G2_R0,
  14, /*clock*/
  12, /*data*/
  U8X8_PIN_NONE
);

// -----------------------------------
// WI-FI Settings
const char *ssid = "**********";
const char *password = "**********";
WiFiServer server(80);

// -----------------------------------
// NTP Settings
// const char *NTPServer = "pool.ntp.org";
// NTPClient timeClient(ntpUDP, ntpServer, 3600); // 3600 seconds = 1 hour offset from UTC

// -----------------------------------
// Helper functions
void setupOLED(void) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_7x14B_tr);
}

void displayOLEDStatus(const char *txt) {
  u8g2.drawStr(0, 10, txt);
  u8g2.sendBuffer();
}

void displayOLEDText(const char *txt) {
  u8g2.drawStr(0, 30, txt);
  u8g2.sendBuffer();
}

void displayStarting(void) {
  u8g2.clearBuffer();
  displayOLEDStatus("Starting...");
  u8g2.sendBuffer();
}

void startWifi(void) {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    u8g2.clearBuffer();
    displayOLEDStatus("Connecting...");
    u8g2.sendBuffer();
  }
}

void printIP(void) {
  displayOLEDStatus(WiFi.localIP().toString().c_str());
  u8g2.sendBuffer();
}

void initTime(void) {
  displayOLEDText("Fetching time...");
  configTzTime("America/Mexico_City", "pool.ntp.org", "time.nist.gov");
  time_t time_now = time(nullptr);
  while (time_now < 24 * 3600) {
    delay(100);
    time_now = time(nullptr);
  }
}


// -----------------------------------
// Start up Code, runs once @ boot
void setup(void) {
  u8g2.begin();         // init OLED display
  setupOLED();          // clear OLED Buffer & set font
  displayStarting();    // Starting message
  startWifi();          // init Wi-Fi
  // initTime();           // fetch time
}

void runWiFiClient(void) {
  // -- START: Wifi Client Stuff -------------------------------------------------------------------
  WiFiClient client = server.available();
  if (!client) {
    return;
  }

  server.begin();
  String req = client.readStringUntil('\r');
  client.flush();

  // Prepare the response
  // String response = clientHTMLResponse();
  String s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
  s += "<!DOCTYPE HTML>\r\n<html>\r\n";
  s += "<pre>";
  s += "Ola k ase? <br>";
  // s += getTimeStr();
  s += "</pre>";
  s += "</html>\n";

  String response = s;
  // Send the response to the client
  client.print(response);
  delay(1);
  // -- END: Wifi Client Stuff -------------------------------------------------------------------
}

String clientHTMLResponse(void) {
  String s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
  s += "<!DOCTYPE HTML>\r\n<html>\r\n";
  s += "<pre>";
  s += "Ola k ase? <br>";
  // s += getTimeStr();
  s += "</pre>";
  s += "</html>\n";
  return s;
}

char* getTimeStr(void) {
  time_t now;
  struct tm timeinfo;
  time(&now);
  localtime_r(&now, &timeinfo);
  return asctime(&timeinfo);
}

void displayTime(void) {
  char* time_str = getTimeStr();
  displayOLEDText(time_str);
}

// -----------------------------------
// Main
void loop(void) {
  delay(1000);

  u8g2.clearBuffer(); // Clean screen to redraw
  printIP();          // Display Server IP @ Status Bar
  // displayTime();      // Display Time

  // runWiFiClient();    // Answer http requests


  // -- START: Wifi Client Stuff -------------------------------------------------------------------
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
  server.begin();

  String req = client.readStringUntil('\r');
  client.flush();

  // Prepare the response
  // String response = clientHTMLResponse();
  String s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
  s += "<!DOCTYPE HTML>\r\n<html>\r\n";
  s += "<pre>";
  s += "Ola k ase? <br>";
  // s += getTimeStr();
  s += "</pre>";
  s += "</html>\n";

  // Send the response to the client
  client.print(s);
  delay(1);
  // -- END: Wifi Client Stuff -------------------------------------------------------------------

  // Serial.printl:willn("Client disonnected");

  // The client will actually be disconnected
  // when the function returns and 'client' object is detroyed


}
