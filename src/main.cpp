#define BLYNK_TEMPLATE_ID "TMPL6qheJEF8G"
#define BLYNK_TEMPLATE_NAME "dadn"
#define BLYNK_AUTH_TOKEN "BheNB766WVkV9YYNkPcOZrNq41eMnNp5"

#include <WiFi.h>
#include <Wire.h>
#include <DHT20.h>
#include <stdint.h>
#include <Adafruit_NeoPixel.h>
#include <BlynkSimpleEsp32.h>
#include <ESP32Servo.h>
#include <IRremote.hpp>
#include "scheduler.h"

#include <HTTPClient.h>

// MACRO
#define fan_pin 32
#define light_pin 33
#define led_pin 27
// #define PIR_PIN 26
#define IR_Pin 19
#define servo_pin 15
#define trig_pin 26
#define echo_pin 18
#define key1_remote 0xF30CFF00
#define key2_remote 0xE718FF00
#define key3_remote 0xA15EFF00
// Prototype Function
void IRAM_ATTR onTimer();
void readDHT20();
void debug();
void on_led(int);
void off_led();
void open_door();
void close_door();
void ultrasonic();
void fan_on(uint8_t pwm = 255);
void fan_off();
hw_timer_s *timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

void sendFanValue(int);
// Variables
const char *ssid = "12345";
const char *pass = "123456789";
bool auto_light_mode = 0;
bool motion_mode = 0;
bool led_state = 0;
bool door_state = 0;
bool fan_state = 0;
// volatile bool motion_detected = false;
long distance = 0;
// unsigned int count = 0;
const char *address = "api.thingspeak.com";
String api_key = "VCRP7KE1JWRSG8ZG";
// object
WiFiClient client;
DHT20 dht20;
ListTask Ltask;
Adafruit_NeoPixel strip(4, led_pin, NEO_GRB + NEO_KHZ800);
Servo door;

void setup()
{
    // --- SET UP PIN ----
    pinMode(fan_pin, OUTPUT);
    pinMode(light_pin, ANALOG);
    pinMode(led_pin, OUTPUT);
    pinMode(IR_Pin, INPUT);
    pinMode(trig_pin, OUTPUT);
    pinMode(echo_pin, INPUT);
    //---- PWM CONFIG -------
    // ledcSetup(1, 1000, 8);
    // ledcAttachPin(fan_pin, 1);

    door.attach(servo_pin);

    Serial.begin(115200);
    Wire.begin();

    //--- TIMER CONFIG -----
    timer = timerBegin(0, 8000, true);
    timerAttachInterrupt(timer, &onTimer, true);
    timerAlarmWrite(timer, 100, true);
    timerAlarmEnable(timer);
    //
    // attachInterrupt(digitalPinToInterrupt(PIR_PIN), detect_motion, RISING);

    // Serial.println("\nScanning I2C devices...");
    // for (byte address = 1; address < 127; address++) {
    //     Wire.beginTransmission(address);
    //     if (Wire.endTransmission() == 0) {
    //         Serial.print("Device found at: 0x");
    //         Serial.println(address, HEX);
    //     }
    // }

    // -----WIFI CONFIG ----
    WiFi.begin(ssid, pass);
    Serial.print("Connecting WiFi...");
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.print("Connected, IP Address: ");
    Serial.println(WiFi.localIP());

    //----- DHT20 CONFIG -------
    dht20.begin();
    //-----REMOTE ------
    IrReceiver.begin(IR_Pin, ENABLE_LED_FEEDBACK);
    // ---- LED ------
    strip.begin();
    strip.show();

    //----Blynk----
    Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

    // ------ADD BEGIN TASK---------
    Ltask.SCH_Add_Task(debug, 3000, 3000);
    Ltask.SCH_Add_Task(readDHT20, 7000, 7000);
    Ltask.SCH_Add_Task(ultrasonic, 2000, 1000);
}

BLYNK_WRITE(V0)
{
    uint8_t button = param.asInt();
    if (button)
    {
        on_led(button);
    }
    else
    {
        off_led();
    }
}
BLYNK_WRITE(V1)
{
    // ledcWrite(1, param.asInt());
    fan_on(param.asInt());
}
BLYNK_WRITE(V4)
{
    auto_light_mode = param.asInt();
}
BLYNK_WRITE(V5)
{
    // Serial.println("Blit");
    motion_mode = param.asInt();
    distance = 0;
}

void loop()
{
    Ltask.SCH_Dispatch_Task();
    Blynk.run();

    if (IrReceiver.decode())
    {
        Serial.println(IrReceiver.decodedIRData.decodedRawData, HEX);
        if (IrReceiver.decodedIRData.decodedRawData == key1_remote)
        {
            if (fan_state)
            {
                Blynk.virtualWrite(V1, 0);
                fan_off();
            }
            else
            {
                Blynk.virtualWrite(V1, 255);
                fan_on();
            }
        }
        else if (IrReceiver.decodedIRData.decodedRawData == key2_remote)
        {
            if (led_state)
            {
                Blynk.virtualWrite(V0, 0);
                off_led();
            }
            else
            {
                Blynk.virtualWrite(V0, 99);
                on_led(99);
            }
        }
        else if (IrReceiver.decodedIRData.decodedRawData == key3_remote)
        {
            if (door_state)
            {
                close_door();
            }
            else
            {
                open_door();
            }
        }
        IrReceiver.resume(); // Enable receiving of the next value
    }

    if (auto_light_mode)
    {
        if (analogRead(light_pin) < 1600)
        {
            Blynk.virtualWrite(V0, 99);
            on_led(99);
        }
        else
            off_led();
    }
    if (motion_mode)
    {
        if (distance < 15 && distance > 1)
        {
            // Serial.println("Detect Motion!!!");
            if (!led_state)
            {
                on_led(99);
                Blynk.virtualWrite(V0, 99); ///
                Ltask.SCH_Add_Task(off_led, 3000, 0);
            }
            if (!door_state)
            {
                open_door();
                Ltask.SCH_Add_Task(close_door, 3000, 0);
            }
            distance = 0;
        }
    }
}

void IRAM_ATTR onTimer()
{
    portENTER_CRITICAL_ISR(&timerMux);
    Ltask.run();
    portEXIT_CRITICAL_ISR(&timerMux);
}
void debug()
{
    Serial.print("TEST SCHEDULER:  ");
    // Serial.println(motion_mode);
    // sendFanValue(101);
}
void readDHT20()
{
    dht20.read();
    float temp = dht20.getTemperature();
    float humi = dht20.getHumidity();
    Serial.print("Temp: ");
    Serial.print(temp);
    Serial.print("°C");
    Serial.print(" - Humidity: ");
    Serial.print(humi);
    Serial.println("%");
    //----send -----
    Blynk.virtualWrite(V2, temp);
    Blynk.virtualWrite(V3, humi);
}
void on_led(int level)
{
    led_state = 1;
    strip.setBrightness(level + 156); // 0 -> 255 ;
    strip.setPixelColor(0, strip.Color(0, 255, 0));
    strip.setPixelColor(1, strip.Color(0, 255, 0));
    strip.setPixelColor(2, strip.Color(0, 255, 0));
    strip.setPixelColor(3, strip.Color(0, 255, 0));
    strip.show();
}
void off_led()
{
    led_state = 0;

    strip.setPixelColor(0, strip.Color(0, 0, 0));
    strip.setPixelColor(1, strip.Color(0, 0, 0));
    strip.setPixelColor(2, strip.Color(0, 0, 0));
    strip.setPixelColor(3, strip.Color(0, 0, 0));
    strip.show();
}
void open_door()
{
    door_state = 1;
    door.write(180);
}
void close_door()
{
    door_state = 0;
    door.write(0);
}
// void IRAM_ATTR detect_motion() {
//     motion_detected = true;
//     count++;
// }
void ultrasonic()
{
    digitalWrite(trig_pin, LOW);
    delayMicroseconds(2);

    digitalWrite(trig_pin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trig_pin, LOW);

    long duration = pulseIn(echo_pin, HIGH, 30000); // Giới hạn 30ms (tương đương ~5m)
    distance = duration * 0.0343 / 2;
    Serial.print("Khoang cach: ");
    Serial.println(distance);
}
void fan_on(uint8_t pwm)
{
    fan_state = 1;
    analogWrite(fan_pin, pwm);
}
void fan_off()
{
    fan_state = 0;
    analogWrite(fan_pin, 0);
}

void sendFanValue(int value)
{
    if (WiFi.status() == WL_CONNECTED)
    {
        HTTPClient http;

        String url = "http://" + String("10.28.129.153") + ":" + String(5000) + "/api/devices/fan/" + String(value);
        http.begin(url);

        int httpResponseCode = http.GET();

        if (httpResponseCode > 0)
        {
            Serial.print("Gửi thành công, mã phản hồi: ");
            Serial.println(httpResponseCode);
            String response = http.getString();
            Serial.println("Phản hồi: " + response);
        }
        else
        {
            Serial.print("Lỗi khi gửi: ");
            Serial.println(httpResponseCode);
        }

        http.end();
    }
    else
    {
        Serial.println("WiFi chưa kết nối!");
    }
}