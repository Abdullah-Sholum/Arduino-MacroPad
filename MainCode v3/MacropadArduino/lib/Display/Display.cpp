#include "Display.h"
#include "idleAnimation.h"

// Definisi variabel 
const unsigned long Display::bootDurations[13] = {800, 800, 800, 800, 800, 800, 800, 800 ,1000, 500, 500, 500, 3000};

// Konstruktor
Display::Display(const int* pins, int count)
  : display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET),
    sliderPins(pins), numSliders(count) {
  sliderValues = new int[numSliders];
  lastSliderValues = new int[numSliders];
  filteredValues = new float[numSliders];
}

// Destructor
Display::~Display() {
  delete[] sliderValues;
  delete[] lastSliderValues;
  delete[] filteredValues;
}

// Inisialisasi OLED dan data awal
void Display::begin() {
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) for (;;);
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  for (int i = 0; i < numSliders; ++i) {
    int v = analogRead(sliderPins[i]);
    sliderValues[i] = v;
    lastSliderValues[i] = v;
    filteredValues[i] = v;
  }

  currentMode = BOOTING;
  bootStep = 0;
  lastBootUpdate = millis();
  textX = -getPuntenWidth();
}

// Loop utama
void Display::update() {
  unsigned long now = millis();
  readSliders(now, currentMode != BOOTING);

  switch (currentMode) {
    case BOOTING: handleBoot(now); break;
    case SHOW_MASTER: handleShowMaster(now); break;
    case SLIDER_CHANGED: handleSliderChanged(now); break;
    case IDLE: handleIdle(now); break;
  }
}

// Membaca semua slider
void Display::readSliders(unsigned long now, bool allowTrigger) {
  bool changed = false;

  for (int i = 0; i < numSliders; ++i) {
    int raw = analogRead(sliderPins[i]);
    filteredValues[i] = (filteredValues[i] * 0.9f) + (raw * 0.1f);
    int value = (int)filteredValues[i];
    sliderValues[i] = value;

    if (allowTrigger && abs(value - lastSliderValues[i]) > changeThreshold) {
      lastSliderValues[i] = value;
      lastChangedIndex = i;
      changed = true;
    }
  }

  if (changed) {
    lastSliderActivity = now;
    currentMode = SLIDER_CHANGED;
  } else if (currentMode == SLIDER_CHANGED && (now - lastSliderActivity) > 4000) {
    enterIdle(now);
  }
}

// Hitung persen dari nilai analog 0–1023
int Display::percentFromRaw(int raw) {
  if (raw <= 0) return 0;
  if (raw >= 1023) return 100;
  long v = (long)raw * 100L + 511L;
  int pct = (int)(v / 1023L);
  return constrain(pct, 0, 100);
}

// Mode boot
void Display::handleBoot(unsigned long now) {
  if (bootStep < 15 && now - lastBootUpdate >= bootDurations[bootStep]) {
    bootStep++;
    lastBootUpdate = now;
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  switch (bootStep) {
    case 0: centerText("M"); break;
    case 1: centerText("MA"); break;
    case 2: centerText("MAC"); break;
    case 3: centerText("MACR"); break;
    case 4: centerText("MACRO"); break;
    case 5: centerText("MACROP"); break;
    case 6: centerText("MACROPA"); break;
    case 7: centerText("MACROPAD"); break;
    case 8: centerText("MACROPAD V3"); break;
    case 9: centerText("Sistem menyala 3"); break;
    case 10: centerText("Sistem menyala 2"); break;
    case 11: centerText("Sistem menyala 1"); break;
    case 12: {
      int master = percentFromRaw(sliderValues[3]);
      String msg = "Master - " + String(master) + "%";
      centerSingleLineText(msg.c_str());
      break;
    }
    default:
      currentMode = SHOW_MASTER;
      lastSliderActivity = now;
      break;
  }

  display.display();
}

// Menampilkan Master
void Display::handleShowMaster(unsigned long now) {
  int master = percentFromRaw(sliderValues[3]);
  display.clearDisplay();
  String msg = "Master - " + String(master) + "%";
  centerSingleLineText(msg.c_str());
  display.display();

  if (now - lastSliderActivity >= 3000) {
    enterIdle(now);
  }
}

// Menampilkan slider yang berubah
void Display::handleSliderChanged(unsigned long now) {
  int idx = lastChangedIndex;
  if (idx < 0) return;

  int pct = percentFromRaw(sliderValues[idx]);
  display.clearDisplay();

  String msg = String(sliderLabels[idx]) + " - " + String(pct) + "%";
  centerSingleLineText(msg.c_str());
  display.display();
}

// Mode idle animasi punten
void Display::handleIdle(unsigned long now) {
  if (now - lastAnimUpdate > animDelay) {
    lastAnimUpdate = now;
    idleAnimation(display); // kirim objek display
  }
}

// Pindah ke idle mode
void Display::enterIdle(unsigned long now) {
  currentMode = IDLE;
  textX = -getPuntenWidth();
  lastAnimUpdate = now;
}

int Display::getPuntenWidth() { return 6 * 6 * 2; }

// Menampilkan teks di tengah layar
void Display::centerText(const char* txt) {
  int16_t x1, y1; uint16_t w, h;
  display.getTextBounds(txt, 0, 0, &x1, &y1, &w, &h);
  int x = (SCREEN_WIDTH - w) / 2;
  int y = (SCREEN_HEIGHT - h) / 2;
  display.setCursor(x, y);
  display.print(txt);
}

// Teks satu baris di tengah (horizontal dan vertikal)
void Display::centerSingleLineText(const char* txt) {
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(txt, 0, 0, &x1, &y1, &w, &h);
  int x = (SCREEN_WIDTH - w) / 2;
  int y = (SCREEN_HEIGHT - h) / 2;
  display.setCursor(x, y);
  display.print(txt);
}
