#include "dht20.h"

DHT20 dht20;

void getValueDHT20() {
    if (dht20.read() == DHT20_OK) {
        Serial.println(String(dht20.getTemperature()) + "-" + String(dht20.getHumidity()));
        publishData("temp", String(dht20.getTemperature()));
        publishData("humi", String(dht20.getHumidity()));
    } else {
        Serial.println("Failed to read DHT20 sensor.");
    }
}

void TaskDHT20(void *pvParameters) {
    while (true) {
        getValueDHT20();
        vTaskDelay(delay_time / portTICK_PERIOD_MS);
    }
}

void initDHT20() {
    Wire.begin(11, 12);
    dht20.begin();
    xTaskCreate(
        TaskDHT20,    // Function to implement the task
        "TaskDHT20",  // Name of the task
        4096,         // Stack size in words
        NULL,         // Task input parameter
        1,            // Priority of the task
        NULL          // Task handle
    );
}