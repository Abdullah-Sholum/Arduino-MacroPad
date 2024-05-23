#include <HID.h>
#include <Keyboard.h>         // Mengimpor pustaka Keyboard untuk mengirim input keyboard
#include <SoftwareSerial.h>   // SoftwareSerial module_bluetooth(0, 1); // pin RX | TX
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

/*switch pull up
  switch terbuka (off) posisi HIGH
  switch tertutup (ON) posisi LOW
switch pull down
  switch terbuka (off) posisi LOW
  switch tertutup (ON) posisi HIGH   
  */
// deklarasi variabel konstanta
const int buz = 2;            // Pin buzzer (D2)
const int ledPwm = 3;         // Pin LED PWM (D3)
const int ledPin = 7;         // Pin LED (D7)
const int switchPin1 = 4;     // Pin switch 1 (D4)
const int switchPin2 = 6;     // Pin switch 2 (D5)
const int NUM_SLIDERS = 6;
const int analogInputs[NUM_SLIDERS] = {A0, A1, A2, A3, A10, A9}; // A3, A10, A9, A8}; nonaktifkan karena pot rusak
int analogSliderValues[NUM_SLIDERS];

//DefinisiDisplay
#define SCREEN_WIDTH 128                  // Lebar layar OLED (128 piksel)
#define SCREEN_HEIGHT 32                  // Tinggi layar OLED (64 piksel)

// Inisialisasi objek display OLED dengan alamat I2C 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);


//animasi sopan


//buat fungsi BuzzerLed
void buzzerLed(int onDelay, int offDelay) {
  digitalWrite(buz, HIGH);              // Nyalakan buzzerz
  digitalWrite(ledPin, HIGH);           // Nyalakan LED
  delay(onDelay);                       // Tunggu sesuai delay buzzer
  digitalWrite(buz, LOW);               // Matikan buzzer
  digitalWrite(ledPin, LOW);            // Matikan LED
  delay(offDelay);                      // Tunggu sesuai delay off
} 

//buat fungsi ledPwmFade
void ledPwmFade() {
  for (int i = 0; i < 3; i++) {                                 // Ulangi proses sebanyak 3 kali
    for (int pwmValue = 0; pwmValue <= 255; pwmValue += 20) {   // Atur nilai PWM dari 0 hingga 255 dengan peningkatan 5
      analogWrite(ledPwm, pwmValue);                           // Set nilai PWM
      delay(30); // Tunggu 30ms
    }
    for (int pwmValue = 255; pwmValue >= 0; pwmValue -= 255) {  // Atur nilai PWM dari 255 ke 0 dengan penurunan -255
      analogWrite(ledPwm, pwmValue);                           // Set nilai PWM
      delay(30); // Tunggu 30ms
    }
  }
}

//buat fungsi switchTest
void switchTest() {
  if (digitalRead(switchPin1) == LOW) {           // Jika switchPin1 dalam keadaan LOW (ditekan)
    digitalWrite(ledPin, HIGH);                   // Hidupkan LEDTest
  } else if (digitalRead(switchPin2) == LOW) {    // Jika switchPin2 dalam keadaan LOW (ditekan)
    digitalWrite(ledPin, HIGH);                   // Hidupkan LEDTest 
  } else {                                        // Jika tidak ada switch ditekan
    digitalWrite(ledPin, LOW);                    // Matikan LEDTest
  }
}

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

//fungsi menampilkan nilai slider ke display
void drawValuePot(String text, int textSize, int numPot, int x, int y) {
  display.clearDisplay();   // Hapus tampilan sebelumnya
  // Atur ukuran teks
  display.setTextSize(textSize);
  // Atur warna teks (putih)
  display.setTextColor(WHITE);
  // Konversi nilai potensio ke rentang 0-100
  int mappedValue = map(analogSliderValues[numPot], 0, 1023, 0, 100);
  // Tampilkan teks nilai potensio dalam rentang 0-100 sesuai dengan nomor potensio dan lokasi yang ditentukan
  display.setCursor(x, y);
  display.println(text + ": ");
  display.setCursor(x, y + textSize * 8);  // Pindah ke baris berikutnya
  display.print(mappedValue);
  display.println(" %");

  // Tampilkan teks yang sudah diatur
  display.display();
  delay(10);
}

//fungsi setup 
void setup() {  
// inisiasi variabel
  pinMode(ledPin, OUTPUT);              // Atur LedPin sebagai OUTPUT
  pinMode(buz, OUTPUT);                 // Atur buz sebagai OUTPUT
  pinMode(ledPwm, OUTPUT);             // Atur ledPwm sebagai OUTPUT
  pinMode(switchPin1, INPUT_PULLUP);    // Atur switchPin1 sebagai INPUT_PULLUP8 
  pinMode(switchPin2, INPUT_PULLUP);    // Atur switchPin2 sebagai INPUT_PULLUP

// Inisialisasi display OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("Gagal menginisialisasi display OLED"));
    for(;;);
  }
  // Panggil fungsi untuk menampilkan tulisan dengan animasi bergerak

// looping buat deklaras & isinisasi pin Analog untuk potensi
  for (int i = 0; i < NUM_SLIDERS; i++) {
      pinMode(analogInputs[i], INPUT);
  }
// inisiasi komunikasi
  Serial.begin(9600);                   // menginisialisasi komunikasi serial pada Arduino dengan kecepatan baudrate 9600 bits per detik
  Keyboard.begin();                     // Memulai komunikasi dengan komputer sebagai perangkat HID

//  panggil fungsi buat checking module
//  buzzerLed(200, 50);         // Nyalakan buzzer dan LED selama 200ms, kemudian matikan selama 50ms
//  buzzerLed(0, 0);            // Tunggu sebentar tanpa melakukan apa-apa
//  buzzerLed(100, 50);         // Nyalakan buzzer selama 100ms, kemudian matikan selama 50ms
//  buzzerLed(0, 0);            // Tunggu sebentar tanpa melakukan apa-apa
//  buzzerLed(100, 50);         // Nyalakan buzzer selama 100ms, kemudian matikan selama 50ms
//  ledPwmFade();
}

//looping utama
void loop() {
  //panggil fungsi switchTest
  switchTest();

  //panggil fungsi untuk mengupdate nilai slider
  updateSliderValues();
  
  //pangguil fungsi untuk mengirim nilai slider
  sendSliderValues();       // Kirim nilai slider ke Serial Monitor (debug)
  
  // Tampilkan nilai potensiometer pada OLED display
  // parameter fungsi("text", text size, numPot, x, y)
  drawValuePot("Master", 2, 0, 0, 0);  
//  drawValuePot("Potensio 2", 1, 1, 0, 10); 
//  drawValuePot("Potensio 3", 1, 2, 0, 20);
  delay(10);
}

// 
