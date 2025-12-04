#ifndef DISPLAY_H
#define DISPLAY_H
#include "IdleAnimation.h"   // 🔹 Import animasi idle di sini

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET -1
#define OLED_ADDR 0x3C

class Display {
  public:
    enum Mode { BOOTING, SHOW_MASTER, SLIDER_CHANGED, IDLE };

    Display(const int* pins, int count);
    ~Display();

    void begin();
    void update();

  private:
    Adafruit_SSD1306 display;
    const int* sliderPins;
    int numSliders;
    int* sliderValues;
    int* lastSliderValues;
    float* filteredValues;

    Mode currentMode;
    int bootStep;
    unsigned long lastBootUpdate;
    static const unsigned long bootDurations[13];

    const int changeThreshold = 8;
    unsigned long lastSliderActivity;
    int lastChangedIndex;

    int textX;
    const int textSpeed = 1;
    const unsigned long animDelay = 80;
    unsigned long lastAnimUpdate;

    const char* sliderLabels[6] = {
      "Premier / Mic",
      "Browser",
      "Foobar",
      "Master",
      "Game",
      "Discord"
    };

    void readSliders(unsigned long now, bool allowTrigger);
    int percentFromRaw(int raw);
    void handleBoot(unsigned long now);
    void handleShowMaster(unsigned long now);
    void handleSliderChanged(unsigned long now);
    void handleIdle(unsigned long now);
    void enterIdle(unsigned long now);
    int getPuntenWidth();
    void centerText(const char* txt);
    void centerSingleLineText(const char* txt);
};

#endif
