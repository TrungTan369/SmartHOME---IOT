#include <WiFi.h>
#include <Wire.h>
#include <DHT20.h>
#include <Adafruit_NeoPixel.h>

// #include <LiquidCrystal_I2C.h>

#define fan_pin 32
#define light_pin 33
#define led_pin 27
// #define ir_pin 1
// #define light 39

const char* ssid = "ACLAB";
const char* pass = "ACLAB2023";

unsigned long time_out = 0;
WiFiClient client;
DHT20 dht20;
Adafruit_NeoPixel strip(4, led_pin, NEO_GRB + NEO_KHZ800);
const char* address = "api.thingspeak.com";
String api_key = "VCRP7KE1JWRSG8ZG";

void setup() {
    pinMode(fan_pin, OUTPUT);
    pinMode(light_pin, ANALOG);
    pinMode(led_pin, OUTPUT);
    Serial.begin(115200);
    Wire.begin(); // i2c 

    Serial.println("\nScanning I2C devices...");
    for (byte address = 1; address < 127; address++) {
        Wire.beginTransmission(address);
        if (Wire.endTransmission() == 0) {
            Serial.print("Device found at: 0x");
            Serial.println(address, HEX);
        }
    }

    WiFi.begin(ssid, pass);
    Serial.print("Đang kết nối WiFi...");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\n✅ Kết nối thành công!");
    Serial.print("📶 Địa chỉ IP: ");
    Serial.println(WiFi.localIP());

    if (dht20.begin() != 0) {
        Serial.println("DHT20 lỗi! Kiểm tra kết nối.");
        //while (1);
    }else{
        Serial.println("DHT20 OK!");
    }
    time_out = millis();

    // --- LED ------
    strip.begin();
    strip.show();  // Tắt LED khi khởi động
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

void loop() {
    String a = "";
    while (Serial.available()) {
        char c = Serial.read();
        a += c;
        delay(10);
    }
    
    if (a.length() > 0) {
        Serial.print(" Nhận dữ liệu từ Serial: ");
        Serial.println(a);
        http_get(a);
        a.trim();
        if(a == "1"){
            Serial.print("Fan ON");
            digitalWrite(fan_pin, HIGH);
        }else{
            Serial.print("Fan OFF");
            digitalWrite(fan_pin, LOW);
        }  
    }
    if(millis() - time_out > 5000){
      time_out = millis(); 
      dht20.read();
      float temp = dht20.getTemperature();
      float humi = dht20.getHumidity();
      Serial.print("Temp: "); Serial.print(temp); Serial.print("°C");
      Serial.print(" - Humidity: "); Serial.print(humi); Serial.println("%");
      // Serial.print("light: ");
      // Serial.println(analogRead(light_pin));
    }
    if(analogRead(light_pin) < 1500){
        strip.setPixelColor(0, strip.Color(0, 255, 0));
        strip.setPixelColor(1, strip.Color(0, 255, 0)); 
        strip.setPixelColor(2, strip.Color(0, 255, 0));  
        strip.setPixelColor(3, strip.Color(0, 255, 0));
        strip.show();
    }else{
        strip.setPixelColor(0, strip.Color(0, 0, 0));
        strip.setPixelColor(1, strip.Color(0, 0, 0));
        strip.setPixelColor(2, strip.Color(0, 0, 0));
        strip.setPixelColor(3, strip.Color(0, 0, 0));
        strip.show();

    }
    // Serial.print("light: ");
    // Serial.println(analogRead(light_pin));
    delay(10);
}
