
#include "TaskWebserver.h"

WebServer server(80);
void handleRoot();
void TaskWebserver(void * );

void initWebserver() {
    server.on("/", handleRoot);
    server.begin();
    Serial.println("HTTP server started");

    xTaskCreate(
        TaskWebserver,
        "WebServerTask",
        4096,
        NULL,
        1,
        NULL
    );
}
void TaskWebserver(void* pvParameters) {
    while (1) {
        server.handleClient();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
void handleRoot() {
    dht20.read();
    float t = dht20.getTemperature();
    float h = dht20.getHumidity();
    Serial.println(t);
    Serial.println(h);
    
    String html = "<!DOCTYPE html><html>";
    html += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
    html += "<style>body { text-align: center; font-family: 'Trebuchet MS', Arial; }";
    html += "table { border-collapse: collapse; width:50%; margin-left:auto; margin-right:auto; }";
    html += "th { padding: 10px; background-color: #0043af; color: white; }";
    html += "tr { border: 1px solid #ddd; padding: 12px; }";
    html += "tr:hover { background-color: #bcbcbc; }";
    html += "td { border: none; padding: 10px; }";
    html += ".sensor { color:white; font-weight: bold; background-color: #bcbcbc; padding: 1px; }";
    html += "</style></head>";

    html += "<body><h1>ESP32 with DHT20</h1>";
    html += "<table><tr><th>MEASUREMENT</th><th>VALUE</th></tr>";
    html += "<tr><td>Temperature</td><td><span class=\"sensor\">" + String(t, 1) + " °C</span></td></tr>";
    html += "<tr><td>Humidity</td><td><span class=\"sensor\">" + String(h, 1) + " %</span></td></tr>";
    html += "</table></body></html>";
    server.send(200, "text/html", html);
}