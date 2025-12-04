#include "IdleAnimation.h"
#include <math.h>

static float t = 0.0f; 

static void drawSlider(Adafruit_SSD1306 &display, int x, int knobY) {
  // Gambar batang slider
  display.drawLine(x, 10, x, 54, SSD1306_WHITE);

  // Pastikan knob tidak keluar area slider (10–54)
  if (knobY < 0) knobY = 0;    // agar bagian atas knob masih terlihat
  if (knobY > 30) knobY = 30;    // agar bagian bawah knob masih terlihat

  // Gambar knob
  display.fillRect(x - 5, knobY - 3, 10, 6, SSD1306_WHITE);
}


void idleAnimation(Adafruit_SSD1306 &display) {
  display.clearDisplay();

  // Batasi osilasi sinus di tengah area slider (10–54)
  int midY = 32;
  int amp = 18;  // amplitudo osilasi
  int slider1Y = midY + amp * sin(t);
  int slider2Y = midY + amp * sin(t + 2.0);
  int slider3Y = midY + amp * sin(t + 4.0);
  int slider4Y = midY + amp * sin(t + 6.0);
  int slider5Y = midY + amp * sin(t + 8.0);
  int slider6Y = midY + amp * sin(t + 10.0);

  drawSlider(display, 20, slider1Y);
  drawSlider(display, 36, slider2Y);
  drawSlider(display, 52, slider3Y);
  drawSlider(display, 68, slider4Y);
  drawSlider(display, 84, slider5Y);
  drawSlider(display, 100, slider6Y);

  display.display();
  t += 0.1f;
}
