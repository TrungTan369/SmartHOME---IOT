#ifndef GLOBALS_H
#define GLOBALS_H

#include <Adafruit_NeoPixel.h>
#include <DHT20.h>
#include <ESP32Servo.h>
#include <PubSubClient.h>
#include <WebServer.h>
#include <WiFi.h>
#include <Wire.h>
#include <stdint.h>

#include "../src/connect/TaskMQTT.h"
#include "../src/connect/TaskWiFi.h"
#include "../src/connect/TaskWebserver.h"
#include "../src/devices/dht20.h"

#define delay_time 10000
#define sda 11
#define scl 12

#endif