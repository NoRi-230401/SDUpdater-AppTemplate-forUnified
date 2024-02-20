// *****************************************************
//  SD-Updater App Template for M5Unified library
//                           2024-02-20  by NoRi
// -----------------------------------------------------
//  sample App :  "app00.bin" 
// *****************************************************
#include <Arduino.h>
#include <SD.h>
#include <M5Unified.h>
#include <ESP32-targz.h>
#include <M5StackUpdater.h>
#define TFCARD_CS_PIN 4

#define APP_NAME "app00"      // app Name
#define APP_BIN  "app00.bin"  // app bin file-name
#define TIMEOUT00 10000       // lobby countdouwn timeout : msec
#define TIMEOUT01 20000       // lobby countdouwn timeout : msec
void App00Disp();
void App00BtnChk();
void setupSDUpdater();

void setup(void)
{
  auto cfg = M5.config();
  M5.begin(cfg);
  setupSDUpdater();

  //-- SETUP YOUR_APP --
  App00Disp();
}

void loop(void)
{
  //-- LOOP YOUR_APP ---
  M5.update();

  App00BtnChk();
  delay(1);
}



void setupSDUpdater()
{
  SDUCfg.setLabelMenu("< Menu"); // load menu.bin
  SDUCfg.setLabelSkip("Skip");   // skip the lobby countdown and run the app
  SDUCfg.setAppName(APP_NAME);   // lobby screen label: application name
  checkSDUpdater(
      SD,           // filesystem (default=SD)
      MENU_BIN,     // path to binary (default=/menu.bin, empty string=rollback only)
      TIMEOUT00,    // wait delay, (default=0, will be forced to 2000 upon ESP.restart() )
      TFCARD_CS_PIN // usually default=4 but your mileage may vary
  );
}

void App00BtnChk()
{
  if (M5.BtnA.wasPressed())
  {
    Serial.println("Will go to lobby screen");
    checkSDUpdater(SD, MENU_BIN, TIMEOUT01, TFCARD_CS_PIN);
    App00Disp();
    delay(500);
  }
  else if (M5.BtnB.wasPressed())
  {
    Serial.println("Will Load menu binary");
    updateFromFS(SD);
    ESP.restart();
  }
  else if (M5.BtnC.wasPressed())
  {
    Serial.println("Will store BIN_FILE to SD");
    saveSketchToFS(SD, APP_BIN);
    App00Disp();
    delay(500);
  }
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
