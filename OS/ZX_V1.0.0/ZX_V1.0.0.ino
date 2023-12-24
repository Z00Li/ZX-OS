// -------------------- Libraries --------------------
#include <Arduino.h>                // necessary arduino library
#include <DFRobotDFPlayerMini.h>    // audio player
#include <SoftwareSerial.h>         // audio transmit and receiver
#include <Wire.h>                   // serial communication with audio transmitter
#include <MPU6050.h>                // gyrosensor and acceleration meter
#include <FastLED.h>                // addressable LED library
#include <EEPROM.h>                 // save to storage


// -------------------- Settings --------------------
#define debug 1                     // set 0 to turn off, set 1 to turn on
#define audio_volume 4              // set between 0 and 30 (using amounts over 5 may not work on usb cable)
#define ls_color Yellow             // set lightsaber color (Yellow, Green, Red, Blue...)


// -------------------- Pins --------------------
#define NUM_LEDS 60                 // set how many leds are in the neopixel chain
#define DATA_PIN 6                  // data pin for leds
SoftwareSerial softSerial(10, 11);  // TX and RX ports on mp3 player
#define BATTERY_PIN A0              // which port the battery positive terminal is located in (USE VOLTAGE DIVIDER IF YOUR BATTERY VOLTAGE EXCEEDS 5V!!!)


// -------------------- Setup --------------------
#define FPSerial softSerial         // rename function
DFRobotDFPlayerMini audio;          // rename function
MPU6050 accelgyro;                  // rename function
CRGB leds[NUM_LEDS];                // define leds function


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


  // IMU check  -- for debug
  if (debug == 1) {
    if (accelgyro.testConnection()) 
      Serial.println(F("MPU6050 OK"));
    else
      Serial.println(F("MPU6050 fail"));
  }

  // IMU initialization
  accelgyro.initialize();
  accelgyro.setFullScaleAccelRange(16);
  accelgyro.setFullScaleGyroRange(250);

}

void loop() {

  if (millis() > 200){
    accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);       

    // find absolute and divide on 100
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

    if (debug == 1){  
      Serial.print("$");
      Serial.print(gyroX);
      Serial.print(" ");
      Serial.print(gyroY);
      Serial.print(" ");
      Serial.print(gyroZ);
      Serial.println(";");
      Serial.print(accelY);
    }

    StateGesture();
  }
}

void StateGesture() {
  if (ls_state == false) {
    if (accelY > 180) {
      delay(100);
      if (accelY < 60) {
        audio.play(power_on);
        for (int line = 0; line < NUM_LEDS; line++) {
          leds[line] = CRGB::ls_color;
          FastLED.show();
        }
        ls_state = true;
      }
    }
  }

  if (ls_state == true){
    int ns = 30;              // neutral state
    if (gyroY >= 170 && gyroX < ns && gyroZ < ns && accelY < ns && accelX < ns && accelZ < ns) {
      audio.play(power_off);
      ls_state = false;
    }
  }
}

void CheckBattery()
{
  value = analogRead(BATTERY_PIN);
  voltage = value * 5.0/1023;
  perc = map(voltage, 3.6, 4.2, 0, 100);

}