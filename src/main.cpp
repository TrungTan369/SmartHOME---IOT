#define BLYNK_TEMPLATE_ID "TMPL6rbmwRxaJ"
#define BLYNK_TEMPLATE_NAME "DADN"
#define BLYNK_AUTH_TOKEN "4r7XA37ejbcuNK8BbOZiD8wm5Wvmd-cB"

#include <WiFi.h>
#include <Wire.h>
#include <DHT20.h>
#include <stdint.h>  
#include <Adafruit_NeoPixel.h>
#include <BlynkSimpleEsp32.h>
#include <ESP32Servo.h>
#include <IRremote.hpp>
#include "scheduler.h"
// #include <LiquidCrystal_I2C.h>

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
void http_get(String);
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
// void IRAM_ATTR detect_motion();
hw_timer_s * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

// Variables
// const char* ssid = "ACLAB";
// const char* pass = "ACLAB2023";
const char* ssid = "ahaha";
const char* pass = "123456789";
const char* address = "api.thingspeak.com";
String api_key = "VCRP7KE1JWRSG8ZG";
String serial_read = "";
bool auto_light_mode = 0;
bool motion_mode = 0;
bool led_state = 0;
bool door_state = 0;
bool fan_state = 0;
// volatile bool motion_detected = false;
long distance = 0;
// unsigned int count = 0;
// object
WiFiClient client;
DHT20 dht20;
ListTask Ltask;
Adafruit_NeoPixel strip(4, led_pin, NEO_GRB + NEO_KHZ800);
Servo door;

void setup() {
    // --- SET UP PIN ----
    pinMode(fan_pin, OUTPUT);
    pinMode(light_pin, ANALOG);
    pinMode(led_pin, OUTPUT);
    // pinMode(PIR_PIN, INPUT);
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
    while (WiFi.status() != WL_CONNECTED) {
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
    Ltask.SCH_Add_Task(debug, 3000 , 3000);
    Ltask.SCH_Add_Task(readDHT20, 7000, 7000);
    Ltask.SCH_Add_Task(ultrasonic, 2000, 1000);
}

BLYNK_WRITE(V0){
    bool button = param.asInt();
    if(button){
        on_led();
    }else{
        off_led();
    }
}
BLYNK_WRITE(V1){
    // ledcWrite(1, param.asInt());
    fan_on(param.asInt());
}
BLYNK_WRITE(V4){
    auto_light_mode = param.asInt();
}
BLYNK_WRITE(V5){
    // Serial.println("Blit");
    motion_mode = param.asInt();
}

void loop() {
    Ltask.SCH_Dispatch_Task();
    Blynk.run();

    if (IrReceiver.decode()) {
        Serial.println(IrReceiver.decodedIRData.decodedRawData, HEX);
        if(IrReceiver.decodedIRData.decodedRawData == key1_remote){
            if(fan_state){
                fan_off();
            }else {
                fan_on();
            }
        }else if(IrReceiver.decodedIRData.decodedRawData == key2_remote){
            if(led_state){
                off_led();
            }else {
                on_led();
            }
        }else if(IrReceiver.decodedIRData.decodedRawData == key3_remote){
            if(door_state){
                close_door();
            }else {
                open_door();
            } 
        }
        IrReceiver.resume(); // Enable receiving of the next value
    }

    while (Serial.available()) {
        char c = Serial.read();
        serial_read += c;
        delay(5);
    }
    
    if (serial_read.length() > 0) {
        http_get(serial_read);
        serial_read.trim();
        if(serial_read == "1"){
            fan_on();
        }else{
            fan_off();
        }
        serial_read = "";
    }
    if(auto_light_mode){
        if(analogRead(light_pin) < 1600 )
            on_led();
        else
            off_led();
    }
    if(motion_mode){
        if (distance < 15 && distance > 1){
            // Serial.println("Detect Motion!!!");
            if(!led_state){
                on_led();
                Ltask.SCH_Add_Task(off_led, 3000, 0);
            }
            if(!door_state){
                open_door();
                Ltask.SCH_Add_Task(close_door, 3000, 0);
            }
            distance = 0;
        }
    }
}

void http_get(String a) {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Wifi lost!!");
        return;
    }

    Serial.println("\n Send to ThingSpeak...");
    
    if (client.connect(address, 80)) {
        String getUrl = "/update?api_key=" + api_key + "&field1=" + a;
        
        client.println("GET " + getUrl + " HTTP/1.1");
        client.println("Host: api.thingspeak.com");
        client.println("Connection: close");
        client.println();

        unsigned long timeout = millis();
        while (client.available() == 0) {
            if (millis() - timeout > 5000) {
                Serial.println(" Timeout: server not respon.");
                client.stop();
                return;
            }
        }
        while (client.available()) {
            String line = client.readStringUntil('\r');
            Serial.print(line);
        }
        
        Serial.println("\n Đóng kết nối.");
        client.stop();
    } else {
        Serial.println(" Lỗi kết nối đến server!");
    }
}
void IRAM_ATTR onTimer(){
    portENTER_CRITICAL_ISR(&timerMux);
    Ltask.run();
    portEXIT_CRITICAL_ISR(&timerMux);
}
void debug(){
    Serial.print("TEST SCHEDULER:  ");
    Serial.println(motion_mode);
}
void readDHT20(){
    dht20.read();
    float temp = dht20.getTemperature();
    float humi = dht20.getHumidity();
    Serial.print("Temp: "); Serial.print(temp); Serial.print("°C");
    Serial.print(" - Humidity: "); Serial.print(humi); Serial.println("%");
    //----send -----
    Blynk.virtualWrite(V2, temp);
    Blynk.virtualWrite(V3, humi);
}
void on_led(){
    led_state = 1;
    strip.setPixelColor(0, strip.Color(0, 255, 0));
    strip.setPixelColor(1, strip.Color(0, 255, 0)); 
    strip.setPixelColor(2, strip.Color(0, 255, 0));  
    strip.setPixelColor(3, strip.Color(0, 255, 0));
    strip.show();
}
void off_led(){
    led_state = 0;
    strip.setPixelColor(0, strip.Color(0, 0, 0));
    strip.setPixelColor(1, strip.Color(0, 0, 0)); 
    strip.setPixelColor(2, strip.Color(0, 0, 0));  
    strip.setPixelColor(3, strip.Color(0, 0, 0));
    strip.show();
}
void open_door(){
    door_state = 1;
    door.write(180);
}
void close_door(){
    door_state = 0;
    door.write(0);
}
// void IRAM_ATTR detect_motion() {
//     motion_detected = true;
//     count++;
// }
void ultrasonic(){
    digitalWrite(trig_pin, LOW);
    delayMicroseconds(2);
    
    digitalWrite(trig_pin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trig_pin, LOW);
    
    long duration = pulseIn(echo_pin, HIGH, 30000);  // Giới hạn 30ms (tương đương ~5m)
    distance = duration * 0.0343 / 2;
    Serial.print("Khoang cach: ");
    Serial.println(distance);
}
void fan_on(uint8_t pwm){
    fan_state = 1;
    analogWrite(fan_pin, pwm);
}
void fan_off(){
    fan_state = 0;
    analogWrite(fan_pin, 0);
}