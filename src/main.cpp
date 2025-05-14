// *******************************************************
//  Hello           by NoRi 2025-04-15
// -------------------------------------------------------
// main.cpp
// *******************************************************
#include <Arduino.h>
#include <SD.h>
#if defined(CARDPUTER)
#include <M5Cardputer.h>
SPIClass SPI2;
#else
#include <M5Unified.h>
#endif
#include <M5StackUpdater.h>

void m5stack_begin();
void SDU_lobby();
void disp(String msg);
//-------------------------------------

void setup()
{
  m5stack_begin();
  SDU_lobby();
  disp("Hello World!");
}

void loop()
{
}

void disp(String msg)
{
#ifdef CARDPUTER
  M5Cardputer.Display.println(msg);
#else
  M5.Display.println(msg);
#endif
}

void m5stack_begin()
{
  int i = 0;
  auto cfg = M5.config();
  cfg.serial_baudrate = 115200;

#if defined(CARDPUTER)
// ------------- CARDPUTER ---------------
  M5Cardputer.begin(cfg, true);
  M5Cardputer.Display.setBrightness(70);
  M5Cardputer.Display.setRotation(1);
  M5Cardputer.Display.fillScreen(TFT_BLACK);
  M5Cardputer.Display.setTextSize(2);

  SPI2.begin(
      M5.getPin(m5::pin_name_t::sd_spi_sclk),
      M5.getPin(m5::pin_name_t::sd_spi_miso),
      M5.getPin(m5::pin_name_t::sd_spi_mosi),
      M5.getPin(m5::pin_name_t::sd_spi_ss));

  while (!SD.begin(M5.getPin(m5::pin_name_t::sd_spi_ss), SPI2) && i < 10)
#else
// ----------- Core2 and CoreS3 ----------
  M5.begin(cfg);
  M5.Display.setBrightness(120);
  M5.Display.setRotation(1);
  M5.Display.fillScreen(BLACK);
  M5.Display.setTextSize(2);

  while (!SD.begin(GPIO_NUM_4, SPI, 25000000) && i < 10)
#endif
  {
    delay(500);
    i++;
  }

  if (i >= 10)
  {
    Serial.println("ERR: SD begin ...");
    disp("ERR: SD begin ...");

    while (true)
      ;
  }
}


// ------------------------------------------
// SDU_lobby : SD_uploader lobby
// ------------------------------------------
// load "/menu.bin" on SD
//    if 'a' or 'BtnA' pressed at booting
// ------------------------------------------
void SDU_lobby()
{
  // CORES3 は、最初からBtnAを押していると認識しないのでメッセージ表示後に押下する
  // CORES2 と CardPuter は、最初から BtnA or 'a' を押していればいい。
#ifdef CORES3
  M5.Display.setCursor(0, M5.Display.height() / 2 - 30 );
  M5.Display.setTextColor(GREEN);
  disp("  Press BtnA to load menu");
  delay(3000);
#endif

#if defined(CARDPUTER)
  M5Cardputer.update();
  if (M5Cardputer.Keyboard.isKeyPressed('a'))
#else
  M5.update();
  if (M5.BtnA.isPressed())
#endif
  {
    updateFromFS(SD, "/menu.bin");
    ESP.restart();

    while (true)
      ;
  }

#ifdef CORES3
  M5.Display.fillScreen(BLACK);
  M5.Display.setCursor(0,0);
  M5.Display.setTextColor(WHITE);
#endif
}
