#include <Wire.h>
#include <VL53L0X.h>
VL53L0X sensor;
#define SDA_PIN 2
#define SCL_PIN 15
#define THRESHOLD   40
int pocitadlo = 0;
#include <Servo.h>
static const int servoPin = 23;
Servo servo1;
touch_pad_t touchPin;
int stav = 0;
long tim = 0;
const String numbers[] = {"11111100", "01100000", "11011010", "11110010", "01100110", "10110110", "10111110", "11100000", "11111110", "11110110"};
const int segmentPins[] = {12, 14,27,26,25,33,32,18};
const int digits[] = {5,17,16,4};
#define ON LOW
#define OFF HIGH
#define don HIGH
#define doff LOW
#define dw digitalWrite
char j = '1';
/* pinout
distance -3v3, gnd, 15, 2
servo - vicc, gnd, 23
display (1 - 12) - 25,26,35,27,32,5,14,17,16,33,12,4 
touch - 13
*/
void setup() {
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);
  sensor.setTimeout(500);
  if (!sensor.init()) {
    Serial.println("Failed to detect and initialize sensor!");
    while (1) {}
  }
  sensor.startContinuous();
  servo1.attach(servoPin);
  for (int i = 0; i < 8; i++) {
    pinMode(segmentPins[i], OUTPUT);
  }
  for (int i = 0; i < 4; i++) {
    pinMode(digits[i], OUTPUT);
    digitalWrite(digits[i], OFF);
  }
}

void loop() {
  touchSleepWakeUpEnable(T4,THRESHOLD);
  Serial.println("im awake");
  //servo1.write(0);
  if (stav == 1){
    tim = millis();
    stav = 0;
  }
  int dist = sensor.readRangeContinuousMillimeters();
  if (dist > 1000){
    pocitadlo++;
    delay(6000);
    if (pocitadlo > 2){
      servo1.write(180); delay(1500); servo1.write(0); delay(1500);
      Serial.println("Going to sleep now");
      stav = 1;
      esp_deep_sleep_start();
    }
  }
  else{
    pocitadlo = 0;
    for (int g = 0; g< 100; g++){
      int c = (millis()-tim)/1000;
      Serial.println("cas"); Serial.print(c);
      int x = 0;
      for (int i=10;i<10001;i*=10){
        if (i == 10){
          Serial.println(c%10);
          
          numb(c%10);
          dig(x);
          delay(1);
          x++;
        }
        else{
          
          Serial.println(10*(c%i-c%(i/10))/i);
          numb(10*(c%i-c%(i/10))/i);
          dig(x);
          delay(1);
          x++;
        }
      }
    }
    delay(1);
  } //dw(segmentPins[0], ON);dw(segmentPins[1], ON);dw(segmentPins[2], ON);dw(segmentPins[3], ON);
  //numb(8);
  


}
void numb(int n) {
  
  int ktora = 0;
  for (char c : numbers[n]){
    if (c == j){
      dw(segmentPins[ktora], ON);
    }
    else{dw(segmentPins[ktora], OFF);}
    ktora++;
  }
}
void dig(int n){
  for (int i = 0;i<4;i++){
    if (i == n){
      dw(digits[i], don);
    }
    else {
      dw(digits[i], doff);
    }
  }
}