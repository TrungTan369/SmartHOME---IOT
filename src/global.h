#ifndef GLOBAL_H
#define GLOBAL_H

#include <WiFi.h>
#include <Wire.h>
#include <DHT20.h>
#include <stdint.h>  
#include <Adafruit_NeoPixel.h>
#include <ESP32Servo.h>
// #include <IRremote.hpp>
#include <AdafruitIO.h>
#include "config.h"
#include "scheduler.h"
#include "AdafruitIO_WiFi.h"

// MACRO
#define fan_pin 32
#define light_pin 33
#define led_pin 27
#define IR_Pin 19
#define servo_pin 15
#define trig_pin 26
#define echo_pin 18
#define key1_remote 0xF30CFF00
#define key2_remote 0xE718FF00
#define key3_remote 0xA15EFF00

#define IO_KEY ""
#define IO_USERNAME ""
// Prototype Function
void handleLED(AdafruitIO_Data *);
void handleFAN(AdafruitIO_Data *);
void IRAM_ATTR onTimer();
void readDHT20();
void debug();
void on_led();
void off_led();
void open_door();
void close_door();
void ultrasonic();
void fan_on(uint8_t pwm = 255);
void fan_off();
extern hw_timer_s * timer;
extern portMUX_TYPE timerMux;

// Variables
extern const char* ssid;
extern const char* pass;
extern bool auto_light_mode;
extern bool motion_mode;
extern bool led_state;
extern bool door_state;
extern bool fan_state;
extern long distance ;
// object
extern WiFiClient client;
extern DHT20 dht20;
extern ListTask Ltask;
extern Adafruit_NeoPixel strip;
extern Servo door;
extern AdafruitIO_WiFi io;
extern AdafruitIO_Feed *led;
extern AdafruitIO_Feed *fan;

#endif