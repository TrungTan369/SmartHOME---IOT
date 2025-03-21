#include "global.h"

hw_timer_t *timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
const char* ssid = "12345";
const char* pass = "123456789";
bool auto_light_mode = 0;
bool motion_mode = 0;
bool led_state = 0;
bool door_state = 0;
bool fan_state = 0;
long distance = 0;

// Object
WiFiClient client;
DHT20 dht20;
ListTask Ltask;
Adafruit_NeoPixel strip(4, led_pin, NEO_GRB + NEO_KHZ800);
Servo door;

// Adafruit IO
AdafruitIO_WiFi io(IO_USERNAME, IO_KEY, ssid, pass);
AdafruitIO_Feed *led = io.feed("led");
AdafruitIO_Feed *fan = io.feed("fan");

void handleLED(AdafruitIO_Data *data) {
      if (data->toInt() == 1) {
          on_led();
      } else {
          off_led();
      }
  }
  void handleFAN(AdafruitIO_Data *data){
      if (data->toInt() == 1) {
          fan_on();
      } else {
          fan_off();
      }  
  }
  void IRAM_ATTR onTimer(){
      portENTER_CRITICAL_ISR(&timerMux);
      Ltask.run();
      portEXIT_CRITICAL_ISR(&timerMux);
  }
  void debug(){
      Serial.print("TEST SCHEDULER:  ");
  }
  void readDHT20(){
      dht20.read();
      float temp = dht20.getTemperature();
      float humi = dht20.getHumidity();
      Serial.print("Temp: "); Serial.print(temp); Serial.print("°C");
      Serial.print(" - Humidity: "); Serial.print(humi); Serial.println("%");
  }
void on_led(){
      led_state = 1;
      led->save(led_state);
      strip.setPixelColor(0, strip.Color(0, 255, 0));
      strip.setPixelColor(1, strip.Color(0, 255, 0)); 
      strip.setPixelColor(2, strip.Color(0, 255, 0));  
      strip.setPixelColor(3, strip.Color(0, 255, 0));
      strip.show();
  }
  void off_led(){
      led_state = 0;
      led->save(led_state);
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
  void ultrasonic(){
      digitalWrite(trig_pin, LOW);
      delayMicroseconds(2);
      
      digitalWrite(trig_pin, HIGH);
      delayMicroseconds(10);
      digitalWrite(trig_pin, LOW);
      
      long duration = pulseIn(echo_pin, HIGH, 30000);
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
