 #include <HID.h>
#include <Keyboard.h>         // Mengimpor pustaka Keyboard untuk mengirim input keyboard
#include <SoftwareSerial.h>   // SoftwareSerial module_bluetooth(0, 1); // pin RX | TX
/*
switch pull up
  switch terbuka (off) posisi HIGH
  switch tertutup (ON) posisi LOW
switch pull down
  switch terbuka (off) posisi LOW
  switch tertutup (ON) posisi HIGH   
  */
char ctrlKey = KEY_LEFT_GUI;

// deklarasi variabel konstanta
const int buz = 2;            // Pin buzzer (D2)
const int ledPwm = 3;         // Pin LED PWM (D3)
const int ledPin = 7;         // Pin LED (D7)
const int switchPin1 = 4;     // Pin switch 1 (D4)
const int switchPin2 = 6;     // Pin switch 2 (D5)
const int potRot = 8;         // Deklarasi pin potensio (D8)
const int potSlide1 = 9;      // deklarasi pin potensio (D6)

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

//fungsi controlPotensio
void controlPotRot() {
  int potValue1 = analogRead(potRot); // Baca nilai potensio
  // Konversi nilai potensio ke rentang nilai PWM (0-255)
  int pwmValue1 = map(potValue1, 0, 1023, 0, 255);
  // Atur nilai PWM pada ledPwm sesuai dengan nilai potensio
  analogWrite(ledPwm, pwmValue1);
}
void controlPotSLide1() {
  int potValue2 = analogRead(potSlide1); // Baca nilai potensio
  // Konversi nilai potensio ke rentang nilai PWM (0-255)
  int pwmValue2 = map(potValue2, 0, 1023, 0, 255);
  // Atur nilai PWM pada ledPwm sesuai dengan nilai potensio
  analogWrite(ledPwm, pwmValue2);
}
//fungsi setup 
void setup() {  
// inisiasi variabel
  pinMode(ledPin, OUTPUT);              // Atur LedPin sebagai OUTPUT
  pinMode(buz, OUTPUT);                 // Atur buz sebagai OUTPUT
  pinMode(ledPwm, OUTPUT);             // Atur ledPwm sebagai OUTPUT
  pinMode(switchPin1, INPUT_PULLUP);    // Atur switchPin1 sebagai INPUT_PULLUP8 
  pinMode(switchPin2, INPUT_PULLUP);    // Atur switchPin2 sebagai INPUT_PULLUP

// inisiasi komunikasi
  Serial.begin(9600);                   // menginisialisasi komunikasi serial pada Arduino dengan kecepatan baudrate 9600 bits per detik
  Keyboard.begin();                     // Memulai komunikasi dengan komputer sebagai perangkat HID

//  panggil fungsi buat checking module
  buzzerLed(200, 50);         // Nyalakan buzzer dan LED selama 200ms, kemudian matikan selama 50ms
  buzzerLed(0, 0);            // Tunggu sebentar tanpa melakukan apa-apa
  buzzerLed(100, 50);         // Nyalakan buzzer selama 100ms, kemudian matikan selama 50ms
  buzzerLed(0, 0);            // Tunggu sebentar tanpa melakukan apa-apa
  buzzerLed(100, 50);         // Nyalakan buzzer selama 100ms, kemudian matikan selama 50ms
  ledPwmFade();
}
void loop() {
  switchTest();
  controlPotRot();
  // controlPotSLide1();
}
