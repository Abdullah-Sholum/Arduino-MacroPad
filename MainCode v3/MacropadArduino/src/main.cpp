#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Display.h>

// ============================================================ Konstanta Global ============================================================
#define NUM_SLIDERS 6
const int analogInputs[NUM_SLIDERS] = {A3, A2, A1, A9, A10, A0};
const int MASTER_INDEX = 0; 


// ================== CLASS: OLEDDisplay =======================
class OledController {
  public:
    enum Mode { BOOTING, SHOW_MASTER, SLIDER_CHANGED, IDLE };

    OledController(const int* pins, int count)
      : display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET),
        sliderPins(pins), numSliders(count) {
      sliderValues = new int[numSliders];
      lastSliderValues = new int[numSliders];
      filteredValues = new float[numSliders];
    }

    ~OledController() {
      delete[] sliderValues;
      delete[] lastSliderValues;
      delete[] filteredValues;
    }

    void begin() {
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

    void update() {
      unsigned long now = millis();
      readSliders(now, currentMode != BOOTING);

      switch (currentMode) {
        case BOOTING: handleBoot(now); break;
        case SHOW_MASTER: handleShowMaster(now); break;
        case SLIDER_CHANGED: handleSliderChanged(now); break;
        case IDLE: handleIdle(now); break;
      }
    }

  private:
    Adafruit_SSD1306 display;
    const int* sliderPins;
    int numSliders;
    int* sliderValues;
    int* lastSliderValues;
    float* filteredValues;

    Mode currentMode = BOOTING;

    // Boot control
    int bootStep = 0;
    unsigned long lastBootUpdate = 0;
    const unsigned long bootDurations[5] = {1000, 500, 500, 500, 3000};

    // Slider change detection
    const int changeThreshold = 8;
    unsigned long lastSliderActivity = 0;
    int lastChangedIndex = -1;

    // Idle animation
    int textX = 0;
    const int textSpeed = 1;
    const unsigned long animDelay = 80;
    unsigned long lastAnimUpdate = 0;

    // slider label mapping
    const char* sliderLabels[6] = {
      "MIC",
      "Browser",
      "Tidal",
      "Master",
      "Game",
      "Discord"
    };

    void readSliders(unsigned long now, bool allowTrigger) {
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

    int percentFromRaw(int raw) {
      // Pastikan raw di-bounds
      if (raw <= 0) return 0;
      if (raw >= 1023) return 100;
      // Kalkulasi dengan pembulatan nearest:
      // (raw * 100) / 1023 dengan rounding -> (raw*100 + 1023/2) / 1023
      long v = (long)raw * 100L + 511L; // 1023/2 ≈ 511
      int pct = (int)(v / 1023L);
      if (pct < 0) pct = 0;
      if (pct > 100) pct = 100;
      return pct;
    }
    
    void handleBoot(unsigned long now) {
      if (bootStep < 5 && now - lastBootUpdate >= bootDurations[bootStep]) {
        bootStep++;
        lastBootUpdate = now;
      }

      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(SSD1306_WHITE);

      switch (bootStep) {
        case 0: centerText("MACROPAD V3"); break;
        case 1: centerText("Sistem menyala 3"); break;
        case 2: centerText("Sistem menyala 2"); break;
        case 3: centerText("Sistem menyala 1"); break;
        case 4: {
          int master = map(sliderValues[3], 0, 1023, 0, 100);
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

    void handleShowMaster(unsigned long now) {
      int master = percentFromRaw(sliderValues[3]);
      display.clearDisplay();

      String msg = "Master - " + String(master) + "%";
      centerSingleLineText(msg.c_str());
      display.display();

      if (now - lastSliderActivity >= 3000) {
        enterIdle(now);
      }
    }

    void handleSliderChanged(unsigned long now) {
      int idx = lastChangedIndex;
      if (idx < 0) return;

      int pct = percentFromRaw(sliderValues[idx]);
      display.clearDisplay();

      String msg = String(sliderLabels[idx]) + " - " + String(pct) + "%";
      centerSingleLineText(msg.c_str());
      display.display();
    }

    void handleIdle(unsigned long now) {
      if (now - lastAnimUpdate > animDelay) {
        lastAnimUpdate = now;
        display.clearDisplay();
        display.setTextSize(2);
        display.setCursor(textX, 10);
        display.print("punten");
        display.display();

        textX += textSpeed;
        if (textX > SCREEN_WIDTH) textX = -getPuntenWidth();
      }
    }

    void enterIdle(unsigned long now) {
      currentMode = IDLE;
      textX = -getPuntenWidth();
      lastAnimUpdate = now;
    }

    int getPuntenWidth() { return 6 * 6 * 2; }

    void centerText(const char* txt) {
      int16_t x1, y1; uint16_t w, h;
      display.getTextBounds(txt, 0, 0, &x1, &y1, &w, &h);
      int x = (SCREEN_WIDTH - w) / 2;
      int y = (SCREEN_HEIGHT - h) / 2;
      display.setCursor(x, y);
      display.print(txt);
    }

    // 🔹 Fungsi baru: teks satu baris di tengah (horizontal & vertikal)
    void centerSingleLineText(const char* txt) {
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
};

// ================== CLASS: SliderController ==================
class SliderController {
  private:
    int numSliders;
    const int* pins;
    int values[NUM_SLIDERS];
    int lastValues[NUM_SLIDERS];
    unsigned long lastReadTime;
    const unsigned long readInterval = 10; // ms
    const int changeThreshold = 2; // ambang untuk deteksi perubahan

  public:
    SliderController(const int* inputPins, int count)
      : numSliders(count), pins(inputPins), lastReadTime(0) {
      for (int i = 0; i < NUM_SLIDERS; i++) {
        values[i] = 0;
        lastValues[i] = 0;
      }
    }

    void init() {
      for (int i = 0; i < numSliders; i++) {
        pinMode(pins[i], INPUT);
        values[i] = analogRead(pins[i]);    // baca awal
        lastValues[i] = values[i];
      }
      lastReadTime = millis();
    }

    void update() {
      unsigned long now = millis();
      if (now - lastReadTime >= readInterval) {
        lastReadTime = now;
        for (int i = 0; i < numSliders; i++) {
          // pembacaan mentah — kalau mau smoothing, ubah di sini
          values[i] = analogRead(pins[i]);
        }
      }
    }

    // cek apakah ada perubahan pada salah satu slider
    bool anyChanged() {
      for (int i = 0; i < numSliders; i++) {
        if (abs(values[i] - lastValues[i]) > changeThreshold) {
          return true;
        }
      }
      return false;
    }

    // update lastValues untuk slider yang berubah (dipanggil setelah mengirim)
    void ackChanges() {
      for (int i = 0; i < numSliders; i++) {
        lastValues[i] = values[i];
      }
    }

    int getValue(int index) {
      if (index < 0 || index >= numSliders) return 0;
      return values[index];
    }

    int getPercent(int index) {
      if (index < 0 || index >= numSliders) return 0;
      return map(values[index], 0, 1023, 0, 100);
    }

    // kirim semua nilai ke serial dalam format Deej "v1|v2|...|vN"
    void sendValues() {
      String out = "";
      for (int i = 0; i < numSliders; i++) {
        out += String(values[i]);
        if (i < numSliders - 1) out += "|";
      }
      Serial.println(out);
      ackChanges(); // konfirmasi bahwa perubahan sudah dikirim
    }

    // debug (opsional)
    void printValues() {
      Serial.print("DBG: ");
      for (int i = 0; i < numSliders; i++) {
        Serial.print(values[i]);
        if (i < numSliders - 1) Serial.print("|");
      }
      Serial.println();
    }
};



// ============================================================ OBJECT GLOBAL ============================================================
SliderController sliders(analogInputs, NUM_SLIDERS);

const int sliderPins[] = {A0, A1, A2, A3, A9, A10};
Display oled(sliderPins, 6);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  oled.begin();
  sliders.init();
}

void loop() {
  // update baca semua slider (non-blocking di dalam kelas)
  sliders.update();

  // jika ada perubahan di salah satu slider -> kirim semua nilai ke Deej
  if (sliders.anyChanged()) {
    sliders.sendValues(); // ini akan memanggil Serial.println("v1|v2|...|v6")
  }

  oled.update();
  

  // tak perlu delay besar; beri sedikit jeda agar MCU tidak 100% sibuk
  delay(5);
}