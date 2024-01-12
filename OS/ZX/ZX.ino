// -------------------- Libraries --------------------
#include <Arduino.h>                      // necessary arduino library
#include <DFRobotDFPlayerMini.h>          // audio player
#include <SoftwareSerial.h>               // audio transmit and receiver
#include <Wire.h>                         // serial communication with audio transmitter
#include <MPU6050.h>                      // gyrosensor and acceleration meter
#include <FastLED.h>                      // addressable LED library
#include <EEPROM.h>                       // save to storage


// -------------------- Settings --------------------
#define debug true                        // set false to turn off, set true to turn on
#define audio_volume 4                    // set between 0 and 30 (using amounts over 20 may not work on usb cable)
#define ls_color Yellow                   // set lightsaber color (Yellow, Green, Red, Blue...)
#define LED_Brightness 80                 // set brightness from 0% to 100%


// -------------------- Pins --------------------
#define NUM_LEDS 60                       // set how many leds are in the neopixel chain
#define DATA_PIN 14                       // data pin for leds
SoftwareSerial softSerial(10, 11);        // TX and RX ports on mp3 player
#define BATTERY_PIN A1                    // which port the battery positive terminal is located in (USE VOLTAGE DIVIDER IF YOUR BATTERY VOLTAGE EXCEEDS 5V!!!)                                                           ---


// -------------------- Setup --------------------
#define FPSerial softSerial               // rename function
DFRobotDFPlayerMini audio;                // rename function
MPU6050 accelgyro;                        // rename function
CRGB leds[NUM_LEDS];                      // define leds function


// -------------------- Sounds --------------------
int power_on = 1;
int power_off = 2;
int hum = 3;
int swing = 4;
int clash = 5;


// -------------------- Variables --------------------
int16_t ax, ay, az;
int16_t gx, gy, gz;
unsigned long ACC, GYR, COMPL;
int gyroX, gyroY, gyroZ, accelX, accelY, accelZ, freq, freq_f = 20;
bool ls_state = false;
int value = 0;
float voltage;
float perc;



void setup() {
  Wire.begin();
  Serial.begin(9600);
  audio.volume(audio_volume);
  FPSerial.begin(9600);
  audio.begin(softSerial);
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  FastLED.setBrightness(map(LED_Brightness, 0, 100, 0, 255));

  if (debug == true) {
    #define dbgln Serial.println();
    #define dbg Serial.print();
  } else {
    #define dbgln;
    #define dbg
  }

  // IMU check  -- for debug
  if (accelgyro.testConnection()) {
      dbgln(F("MPU6050 OK"));
  } else {
    dbgln(F("MPU6050 fail"));
  }

  // IMU initialization
  accelgyro.initialize();
  accelgyro.setFullScaleAccelRange(16);
  accelgyro.setFullScaleGyroRange(250);

  // Battery check percent
  CheckBattery();
}

void loop() {
  delay(200);

  dbg("$");
  dbg(gyroX);
  dbg(" ");
  dbg(gyroY);
  dbg(" ");
  dbg(gyroZ);
  dbgln(";");
  dbg(accelY);

  StateGesture();
  if (ls_state = true)
  {
    run();
  }
}

void GetVector() {
    accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);       

    // find absolute value and divide on 100
    gyroX = abs(gx / 100);
    gyroY = abs(gy / 100);
    gyroZ = abs(gz / 100);
    accelX = abs(ax / 100);
    accelY = abs(ay / 100);
    accelZ = abs(az / 100);

    // vector sum
    ACC = sq((long)accelX) + sq((long)accelY) + sq((long)accelZ);
    ACC = sqrt(ACC);
    GYR = sq((long)gyroX) + sq((long)gyroY) + sq((long)gyroZ);
    GYR = sqrt((long)GYR);
    COMPL = ACC + GYR;
}

void StateGesture() {
  if (ls_state == false) {
    if (accelY > 180 && accelX < 30 && accelZ < 30) {
      dbg("StateGestures 1");
      GetVector();
      if (accelY < 60) {
        audio.play(power_on);
        for (int line = 0; line < NUM_LEDS; line++) {
          leds[line] = CRGB::ls_color;
          FastLED.show();
        }
        ls_state = true;
      }
    }
  } else {
    int ns = 30;                          // neutral state
    if (gyroY >= 170 && gyroX < ns && gyroZ < ns && accelY < ns && accelX < ns && accelZ < ns) {
      delay(100);
      if (gyroY < ns && gyroX < ns && gyroZ < ns && accelY < ns && accelX < ns && accelZ < ns) {
        ls_state = false;
        audio.play(power_off);
      }
    }
  }
}

void CheckBattery()
{
  value = analogRead(BATTERY_PIN);        // check battery
  voltage = value * 5.0/1023;
  perc = map(voltage, 3.6, 4.2, 0, 100);

  if (debug == 1) {                       // battery percent for debug
    Serial.println("Battery voltage: ");
    Serial.print(voltage);
    Serial.println("Battery percent: ");
    Serial.print(perc);
  }

  int ledsToLight = map(perc, 0, 100, 0, NUM_LEDS);   // light leds, to match battery percent
  for (int led = 0; led < ledsToLight; led++) {
    leds[led] = CRGB::ls_color;
  }
  FastLED.show();
  delay(3000);
  for (int led = 0; led < NUM_LEDS; led ++) {         // turn off the leds after 3 seconds
    leds[led] = CRGB::Black;
  }
  FastLED.show();
}

void run()
{
  audio.play(3);
  if (gyroX > 150 || gyroY > 150 || gyroZ > 150)
  {
    audio.play(4);
  }
}