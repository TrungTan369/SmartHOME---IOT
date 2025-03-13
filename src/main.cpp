#define BLYNK_TEMPLATE_ID "TMPL6rbmwRxaJ"
#define BLYNK_TEMPLATE_NAME "DADN"
#define BLYNK_AUTH_TOKEN "4r7XA37ejbcuNK8BbOZiD8wm5Wvmd-cB"

#include <WiFi.h>
#include <Wire.h>
#include <DHT20.h>
#include <Adafruit_NeoPixel.h>
#include <BlynkSimpleEsp32.h>
#include "scheduler.h"
// #include <LiquidCrystal_I2C.h>

// MACRO
#define fan_pin 32
#define light_pin 33
#define led_pin 27
#define PIR_PIN 2

// Prototype Function
void http_get(String);
void IRAM_ATTR onTimer();
void readDHT20();
void debug();
void on_led();
void off_led();

hw_timer_s * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

// Variables
const char* ssid = "ACLAB";
const char* pass = "ACLAB2023";
const char* address = "api.thingspeak.com";
String api_key = "VCRP7KE1JWRSG8ZG";
String serial_read = "";
bool auto_light_mode = 0;
bool motion_mode = 0;
// object
WiFiClient client;
DHT20 dht20;
ListTask Ltask;
Adafruit_NeoPixel strip(4, led_pin, NEO_GRB + NEO_KHZ800);
// WidgetLED led_blynk(V0);

void setup() {
    // --- SET UP PIN ----
    // pinMode(fan_pin, OUTPUT);
    pinMode(light_pin, ANALOG);
    pinMode(led_pin, OUTPUT);
    pinMode(PIR_PIN, INPUT);
    
    //---- PWM CONFIG -------
    ledcSetup(0, 5000, 8);  // Channel 0, tần số 5kHz, độ phân giải 8-bit (0-255)
    ledcAttachPin(fan_pin, 0);  // Gán GPIO32 vào kênh PWM 0

    Serial.begin(115200);
    Wire.begin();

    //--- TIMER CONFIG -----
    timer = timerBegin(0, 8000, true);
    timerAttachInterrupt(timer, &onTimer, true);
    timerAlarmWrite(timer, 100, true);
    timerAlarmEnable(timer);

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
    Serial.print("Đang kết nối WiFi...");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.print(" Kết nối thành công, Địa chỉ IP: ");
    Serial.println(WiFi.localIP());

    //----- DHT20 CONFIG -------
    if (dht20.begin() != 0) {
        Serial.println("DHT20 lỗi! Kiểm tra kết nối.");
    }
    else{
        Serial.println("DHT20 OK!");
    }
    // ---- LED ------
    strip.begin();
    strip.show();

    //----Blynk----
    Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

    // ------ADD BEGIN TASK---------
    Ltask.SCH_Add_Task(debug, 3000 , 3000);
    Ltask.SCH_Add_Task(readDHT20, 7000, 7000);
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
    ledcWrite(0, param.asInt());
}
BLYNK_WRITE(V4){
    auto_light_mode = param.asInt();
}
BLYNK_WRITE(V5){
    motion_mode = param.asInt();
}

void loop() {
    Ltask.SCH_Dispatch_Task();
    Blynk.run();
    while (Serial.available()) {
        char c = Serial.read();
        serial_read += c;
        delay(5);
    }
    
    if (serial_read.length() > 0) {
        http_get(serial_read);
        serial_read.trim();
        if(serial_read == "1"){
            digitalWrite(fan_pin, 100);
        }else{
            digitalWrite(fan_pin, LOW);
        }
        serial_read = "";
    }
    if(auto_light_mode){
        if(analogRead(light_pin) < 1500 )
            on_led();
        else
            off_led();
    }
    if(motion_mode){
        if (digitalRead(PIR_PIN))
            on_led();
        else
            off_led();
    }
    delay(1);
}


void http_get(String a) {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Wifi lost!!");
        return;
    }

    Serial.println("\n Đang gửi dữ liệu lên ThingSpeak...");
    
    if (client.connect(address, 80)) {
        Serial.println(" Kết nối server thành công!");
        String getUrl = "/update?api_key=" + api_key + "&field1=" + a;
        
        client.println("GET " + getUrl + " HTTP/1.1");
        client.println("Host: api.thingspeak.com");
        client.println("Connection: close");
        client.println();

        unsigned long timeout = millis();
        while (client.available() == 0) {
            if (millis() - timeout > 5000) {
                Serial.println(" Timeout: Không nhận phản hồi từ server.");
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
    Serial.println("TEST SCHEDULER");
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
    strip.setPixelColor(0, strip.Color(0, 255, 0));
    strip.setPixelColor(1, strip.Color(0, 255, 0)); 
    strip.setPixelColor(2, strip.Color(0, 255, 0));  
    strip.setPixelColor(3, strip.Color(0, 255, 0));
    strip.show();
}
void off_led(){
    strip.setPixelColor(0, strip.Color(0, 0, 0));
    strip.setPixelColor(1, strip.Color(0, 0, 0)); 
    strip.setPixelColor(2, strip.Color(0, 0, 0));  
    strip.setPixelColor(3, strip.Color(0, 0, 0));
    strip.show();
}