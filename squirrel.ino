#include <ESP8266WiFi.h>
#include <Wire.h>
#include <U8g2lib.h>

#include <TZ.h>       // TimeZones
#include <time.h>     // time() ctime()
#include <sys/time.h> // struct timeval
#include <sntp.h>     // sntp_servermode_dhcp()

// ------------------------------------------------------
// Time Settings
#define TIME_ZONE TZ_America_Mexico_City
// ------------------------------------------------------
// OLED Display
U8G2_SSD1306_128X64_NONAME_F_SW_I2C
u8g2(
    U8G2_R0,
    14, /*clock*/
    12, /*data*/
    U8X8_PIN_NONE);
// ------------------------------------------------------
// WI-FI Settings
const char *ssid = "<SSID>";
const char *password = "<PASSWORD>";
WiFiServer server(80);

// ------------------------------------------------------
// Helper functions
void initOLED(void)
{
  u8g2.begin();                     // init OLED display
  u8g2.clearBuffer();               // Clean Screen
  u8g2.setFont(u8g2_font_7x14B_tr); // Set Font -> https://github.com/olikraus/u8g2/wiki/fntlistallplain
}

void displayOLEDStatus(const char *txt)
{
  u8g2.setDrawColor(0);        // Set color to background (black)
  u8g2.drawBox(0, 0, 140, 14); // Draw a black box starting at 0,0
  u8g2.setDrawColor(1);        // Set color to foreground (yellow/blue)
  u8g2.drawStr(0, 10, txt);    // Draw the passed String
  u8g2.sendBuffer();           // Display the buffer
}

void displayOLEDText(const char *txt)
{
  u8g2.drawStr(0, 30, txt);
  u8g2.sendBuffer();
}

void initWiFi(void)
{
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    displayOLEDStatus("Connecting...");
  }
}

void displayIP(void)
{
  displayOLEDStatus(WiFi.localIP().toString().c_str());
}

void initTime(void)
{
  displayOLEDText("Fetching time...");

  configTzTime(TIME_ZONE, "pool.ntp.org", "time.nist.gov"); // Configure Time Zone

  time_t time_now = time(nullptr);
  while (time_now < 24 * 3600)
  {
    delay(100);
    time_now = time(nullptr);
  }
}

// void runWiFiClient(void) {
//   // -- START: Wifi Client Stuff -------------------------------------------------------------------
//   WiFiClient client = server.available();
//   if (!client) {
//     return;
//   }

//   String req = client.readStringUntil('\r');
//   client.flush();

//   // Prepare the response
//   // String response = clientHTMLResponse();
//   String s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
//   s += "<!DOCTYPE HTML>\r\n<html>\r\n";
//   s += "<pre>";
//   s += "Ola k ase? <br>";
//   // s += getTimeStr();
//   s += "</pre>";
//   s += "</html>\n";

//   String response = s;
//   // Send the response to the client
//   client.print(response);
//   delay(1);
//   // -- END: Wifi Client Stuff -------------------------------------------------------------------
// }

// String clientHTMLResponse(void) {
//   String s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
//   s += "<!DOCTYPE HTML>\r\n<html>\r\n";
//   s += "<pre>";
//   s += "Ola k ase? <br>";
//   // s += getTimeStr();
//   s += "</pre>";
//   s += "</html>\n";
//   return s;
// }

// char* getTimeStr(void) {
//   time_t now;
//   struct tm timeinfo;
//   time(&now);
//   localtime_r(&now, &timeinfo);
//   return asctime(&timeinfo);
// }

void displayTime(void)
{
  // Combined screen render: status bar (IP) + formatted time
  time_t t = time(nullptr);
  struct tm timeinfo;
  localtime_r(&t, &timeinfo);

  char timestr[32];
  strftime(timestr, sizeof(timestr), "%H:%M:%S @ %d.%m.%y", &timeinfo);
  // char datestr[32];
  // strftime(timestr, sizeof(timestr), "%d:%m:%Y", &timeinfo);

  // Prepare IP string
  String ip = WiFi.localIP().toString();

  // Draw everything into buffer then send once to avoid clobbering
  u8g2.clearBuffer();

  // Draw status bar
  u8g2.setDrawColor(0);
  u8g2.drawBox(0, 0, 128, 14);
  u8g2.setDrawColor(1);
  u8g2.setFont(u8g2_font_7x14B_tr);
  u8g2.drawStr(0, 10, ip.c_str());

  // Draw time in the main area
  u8g2.setFont(u8g2_font_6x13B_tr);
  u8g2.drawStr(0, 30, timestr);

  // Calculate days until next 14th (this month if day<=14, otherwise next month)
  struct tm today_mid = timeinfo;
  today_mid.tm_hour = 0;
  today_mid.tm_min = 0;
  today_mid.tm_sec = 0;
  time_t today_mid_t = mktime(&today_mid);

  struct tm target = today_mid;
  if (timeinfo.tm_mday <= 14)
  {
    target.tm_mday = 14;
  }
  else
  {
    target.tm_mon += 1; // next month
    target.tm_mday = 14;
  }
  // Normalize target (mktime will adjust year if month overflows)
  time_t target_t = mktime(&target);

  long days_until = (target_t - today_mid_t) / 86400; // seconds in a day
  if (days_until < 0)
    days_until = 0;

  char faltan[32];
  snprintf(faltan, sizeof(faltan), "Faltan: %ld dias", days_until);
  u8g2.setFont(u8g2_font_7x14B_tr);
  u8g2.drawStr(0, 45, faltan);

  u8g2.sendBuffer();
}

// ------------------------------------------------------
// Start up Code, runs once @ boot
void setup(void)
{
  initOLED(); // clear OLED Buffer & set font
  displayOLEDStatus("Starting...");
  initWiFi(); // init Wi-Fi
  initTime(); // fetch time
}

// -----------------------------------
// Main
void loop(void)
{
  delay(1000);
  // displayIP();

  displayTime(); // Display Server IP + Time on OLED

  // runWiFiClient();    // Answer http requests

  // -- START: Wifi Client Stuff -------------------------------------------------------------------
  // WiFiClient client = server.available();
  // if (!client) {
  //   return;
  // }
  // while (client.available()) {
  //   delay(10);
  // }

  // String req = client.readStringUntil('\r');

  // // Prepare the response
  // // String response = clientHTMLResponse();
  // String s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE html><html>\r\n";
  // s += "<pre>";
  // s += "Ola k ase?";
  // s += "</pre>";
  // s += "</html>\n";

  // // Send the response to the client
  // client.print(s);
  // delay(1);
  // client.flush();
  // // -- END: Wifi Client Stuff -------------------------------------------------------------------

  // Serial.printl:willn("Client disonnected");

  // The client will actually be disconnected
  // when the function returns and 'client' object is detroyed
}
