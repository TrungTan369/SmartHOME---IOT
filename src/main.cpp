#include "globals.h"

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("TEST");
    initDHT20();
    initWiFi();
    initMQTT();
    initWebserver();
}

void loop() {
    if (!Wifi_reconnect()) {
        return;
    }
    reconnectMQTT();
}