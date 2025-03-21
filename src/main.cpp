#include "global.h"
#include <IRremote.hpp>

void remote();

void setup() {
    // --- SET UP PIN ----
    pinMode(fan_pin, OUTPUT);
    pinMode(light_pin, ANALOG);
    pinMode(led_pin, OUTPUT);
    pinMode(IR_Pin, INPUT);
    pinMode(trig_pin, OUTPUT);
    pinMode(echo_pin, INPUT);

    door.attach(servo_pin);

    Serial.begin(115200);
    Wire.begin();

    //--- TIMER CONFIG -----
    timer = timerBegin(0, 8000, true);
    timerAttachInterrupt(timer, &onTimer, true);
    timerAlarmWrite(timer, 100, true);
    timerAlarmEnable(timer);
    //
    io.connect();
    led->onMessage(handleLED);
    fan->onMessage(handleFAN);
 
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

    // ------ADD BEGIN TASK---------
    Ltask.SCH_Add_Task(debug, 3000 , 3000);
    Ltask.SCH_Add_Task(readDHT20, 7000, 7000);
    Ltask.SCH_Add_Task(ultrasonic, 2000, 1000);
    Ltask.SCH_Add_Task(remote, 1000, 100);
}
void loop() {
    Ltask.SCH_Dispatch_Task();
    io.run();
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
void remote(){
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
}


    // while (Serial.available()) {
    //     char c = Serial.read();
    //     serial_read += c;
    //     delay(5);
    // }
    
    // if (serial_read.length() > 0) {
    //     http_get(serial_read);
    //     serial_read.trim();
    //     if(serial_read == "1"){
    //         fan_on();
    //     }else{
    //         fan_off();
    //     }
    //     serial_read = "";
    // }