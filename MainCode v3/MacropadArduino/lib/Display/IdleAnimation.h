#ifndef IDLE_ANIMATION_H
#define IDLE_ANIMATION_H

#pragma once
#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


// Metadata animasi
#define IDLE_FRAME_DELAY 42
#define IDLE_FRAME_WIDTH 64
#define IDLE_FRAME_HEIGHT 64
#define IDLE_FRAME_COUNT 8 // jumlah frame animasi

// Deklarasi fungsi
void drawIdleAnimation(Adafruit_SSD1306 &display);
void idleAnimation(Adafruit_SSD1306 &display);

#endif
