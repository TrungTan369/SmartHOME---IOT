#include "TaskWifi.h"

const char WIFI_SSID[] = "ACLAB";
const char WIFI_PASSWORD[] = "ACLAB2023";

void initWiFi() {
    Serial.println("Connecting to AP ...");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("Connected to AP");
}

bool Wifi_reconnect() {
    const wl_status_t status = WiFi.status();
    if (status == WL_CONNECTED) {
        return true;
    }
    initWiFi();
    return true;
}