// *****************************************************
//  SD-Updater App Template for M5Unified library
//                           2024-02-22  by NoRi
// -----------------------------------------------------
//  sample App :  "app00.bin"
// *****************************************************
#include <Arduino.h>
#include <SD.h>
#include <M5Unified.h>
#include <ESP32-targz.h>
#include <M5StackUpdater.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <SPIFFS.h>
#include <LITTLEFS.h>
#include <ESPWebDAV.h>

#define APP_VER "SDU-webDav-230619"
#define APP_NAME "SDU-webDav"    // app Name
#define APP_BIN "/00_webDav.bin" // app bin file-name
#define TIMEOUT00 10000          // lobby countdouwn timeout : msec
#define STASSID "aterm-d5c4c3-g" // ご自分のルータSSID
#define STAPSK "86c71a78ea6e1"   // ご自分のルータPassword
#define HOSTNAME "stackchan"

void App00Disp();
void App00BtnChk();
void setupSDUpdater(const char *appName);
int SDCARD_CS_PIN = 4;
void setupServer();
void loopServer();
void setupSTA();
void setupDisp();

void setup(void)
{
  auto cfg = M5.config();
  M5.begin(cfg);
  // ------------------------------------------------------
  M5.Log.println(APP_VER);
  SDCARD_CS_PIN = (int)M5.getPin(m5::sd_spi_cs);
  // M5.Log.printf("SDCARD_CS_PIN = %d\n", SDCARD_CS_PIN);
  setupSDUpdater(APP_NAME);
  // ------------------------------------------------------

  setupDisp();
  SD.begin(SDCARD_CS_PIN, SPI, 4000000U);

  // SPIFFS begin
  if (!SPIFFS.begin())
  {
    // SPIFFS is unformatted
    Serial.print("SPIFFS Format ... (please wait)");
    delay(100);
    SPIFFS.format();
    Serial.println("Down");
    delay(3000);
    ESP.restart();
  }

  setupServer();

  // NTP begin
  configTime(9 * 3600, 0, "pool.ntp.org");
  struct tm timeInfo;
  if (getLocalTime(&timeInfo))
  {
    Serial.print("LTm:");
    Serial.printf("%04d-%02d-%02d ", timeInfo.tm_year + 1900, timeInfo.tm_mon + 1, timeInfo.tm_mday);
    Serial.printf("%02d:%02d:%02d\n", timeInfo.tm_hour, timeInfo.tm_min, timeInfo.tm_sec);

    M5.Display.print("LTm:");
    M5.Display.printf("%04d-%02d-%02d ", timeInfo.tm_year + 1900, timeInfo.tm_mon + 1, timeInfo.tm_mday);
    M5.Display.printf("%02d:%02d:%02d\n", timeInfo.tm_hour, timeInfo.tm_min, timeInfo.tm_sec);
  }

  M5.Display.println("setup done !");
  M5.Log.println("setup done !");
}

void loop(void)
{
  //-- LOOP YOUR_APP ---
  M5.update();

  loopServer();
  delay(1);
}

void setupSDUpdater(const char *appName)
{

  SDUCfg.setLabelMenu("< Menu"); // load menu.bin
  SDUCfg.setLabelSkip("Skip");   // skip the lobby countdown and run the app
  SDUCfg.setAppName(appName);    // lobby screen label: application name
  checkSDUpdater(
      SD,        // filesystem (default=SD)
      MENU_BIN,  // path to binary (default=/menu.bin, empty string=rollback only)
      TIMEOUT00, // wait delay, (default=0, will be forced to 2000 upon ESP.restart() )
      SDCARD_CS_PIN);
}

void App00BtnChk()
{
  // if (M5.BtnA.wasPressed())
  if (M5.BtnA.wasClicked())
  {
    M5.Log.println("Will go to lobby screen");
    setupSDUpdater(APP_NAME);
    App00Disp();
    delay(500);
  }
  // else if (M5.BtnB.wasPressed())
  else if (M5.BtnB.wasClicked())
  {
    M5.Log.println("Will Load menu binary");
    updateFromFS(SD);
    ESP.restart();
  }
  // else if (M5.BtnC.wasPressed())
  else if (M5.BtnC.wasClicked())
  {
    M5.Log.println("Will store BIN_FILE to SD");
    saveSketchToFS(SD, APP_BIN);
    App00Disp();
    delay(500);
  }
}

void setupDisp()
{
  M5.Display.setTextFont(1);
  M5.Display.setTextSize(2);
  M5.Display.setTextColor(WHITE, BLACK);
  M5.Display.setTextDatum(0);
  M5.Display.setCursor(0, 0);
  M5.Display.fillScreen(BLACK);
  M5.Display.setTextScroll(true);

  M5.Display.print(" *** SDU-webDav *** \n\n");
}

void App00Disp()
{
  M5.Display.setTextFont(1);
  M5.Display.setTextSize(2);
  M5.Display.setTextColor(WHITE, BLACK);
  M5.Display.setTextDatum(0);
  M5.Display.setCursor(0, 0);
  M5.Display.fillScreen(BLACK);

  M5.Display.print("*** app00 *** \n\n\n\n");
  M5.Display.print("BtnA: lobby screen appear\n\n");
  M5.Display.print("BtnB: load menu binary\n\n");
  M5.Display.print("BtnC: store BIN file to SD\n");
}


void setupSTA()
{
  WiFi.persistent(true);
  WiFi.setHostname(HOSTNAME);
  WiFi.mode(WIFI_STA);
  Serial.begin(115200);
  WiFi.begin(STASSID, STAPSK);
  Serial.println("Connecting to " STASSID " ...");
  M5.Display.println("Connecting to " STASSID " ...");

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
    M5.Display.print(".");
  }
  // Serial.println("");
  Serial.printf("\nConnected !\n");
  // Serial.println(STASSID);
  Serial.print("IP_Addr: ");
  Serial.println(WiFi.localIP());
  Serial.print("RSSI: ");
  Serial.println(WiFi.RSSI());

  // M5.Display.println("");
  M5.Display.printf("\nConnected !\n");
  // M5.Display.println(STASSID);
  M5.Display.print("IP_Addr: ");
  M5.Display.println(WiFi.localIP());
  M5.Display.print("RSSI: ");
  M5.Display.println(WiFi.RSSI());
}

FS &gfs = SD;

WiFiServer tcp(80);
ESPWebDAV dav;

void setupServer()
{
  setupSTA();

  MDNS.begin(HOSTNAME);
  tcp.begin();
  dav.begin(&tcp, &gfs);
  dav.setTransferStatusCallback([](const char *name, int percent, bool receive)
                                {
                                  Serial.printf("%s:%s:%d%%\n", receive ? "recv" : "send", name, percent);
                                  M5.Display.printf("%s:%s:%d%%\n", receive ? "recv" : "send", name, percent); });

  M5.Log.printf("HOSTNAME: %s\n", HOSTNAME);
  M5.Log.println("Server Started !");
  // M5.Display.printf("HOSTNAME: %s\n", HOSTNAME);
  // M5.Display.println("Server Started !");

  Serial.println();
  Serial.print("IP_Addr: ");
  Serial.println(WiFi.localIP());

  // Serial.print("WebDav: file://");
  // Serial.print(WiFi.localIP());
  // Serial.println("/DavWWWRoot");

  Serial.print("file://");
  Serial.print(HOSTNAME);
  Serial.println("/DavWWWRoot");

  M5.Display.print("file://");
  M5.Display.print(HOSTNAME);
  M5.Display.println("/DavWWWRoot");
}

void loopServer()
{
  dav.handleClient();
}
