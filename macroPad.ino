// #include <HID.h>
#include <Keyboard.h>         
#include <SoftwareSerial.h>  
#include <SPI.h>
#include <Wire.h>
// #include <Adafruit_GFX.h>
// #include <Adafruit_SSD1306.h>

const int NUM_SLIDERS = 6;
const int analogInputs[NUM_SLIDERS] = {A3, A2, A1, A9, A10, A0}; 
int analogSliderValues[NUM_SLIDERS];


//buat fungsi update nilai slider
void updateSliderValues() {
  for (int i = 0; i < NUM_SLIDERS; i++) {
     analogSliderValues[i] = analogRead(analogInputs[i]);
  }
}

//buat fungsi untuk mengirim nilai slider
void sendSliderValues() {
  String builtString = String("");
  for (int i = 0; i < NUM_SLIDERS; i++) {
    builtString += String((int)analogSliderValues[i]);
    if (i < NUM_SLIDERS - 1) {
      builtString += String("|");
    }
  }
  Serial.println(builtString);
}

//buat fungsi untuk print nilai slider
void printSliderValues() {
  for (int i = 0; i < NUM_SLIDERS; i++) {
    String printedString = String("Slider #") + String(i + 1) + String(": ") + String(analogSliderValues[i]) + String(" mV");
    Serial.write(printedString.c_str());
    if (i < NUM_SLIDERS - 1) {
      Serial.write(" | ");
    } else {
      Serial.write("\n");
    }
  }
}

//fungsi setup 
void setup() {  

// looping buat deklaras & isinisasi pin Analog untuk potensi
  for (int i = 0; i < NUM_SLIDERS; i++) {
      pinMode(analogInputs[i], INPUT);
  }
// inisiasi komunikasi
  Serial.begin(9600);                 
  Keyboard.begin();                     
}

//looping utama
void loop() {


  //panggil fungsi untuk mengupdate nilai slider
  updateSliderValues();
  
  //panggil fungsi untuk mengirim nilai slider
  sendSliderValues();      
  delay(10);
}

// 
