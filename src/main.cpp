#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "config.h"
#include "display.h"
#include "mqtt_client.h"

SolarData  data    = {};
Display    display;
MqttClient mqtt(data);

unsigned long lastRefresh = 0;

void connectWifi() {
    display.showStatus("Connecting WiFi", WIFI_SSID);
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("WiFi connecting");
    while (WiFi.status() != WL_CONNECTED) {
        delay(250);
        Serial.print(".");
    }
    Serial.print(" OK — ");
    Serial.println(WiFi.localIP());
    display.showStatus("WiFi OK", WiFi.localIP().toString().c_str());
    delay(600);
}

void setup() {
    Serial.begin(115200);
    display.begin();
    connectWifi();
    display.showStatus("MQTT connecting", MQTT_HOST);
    mqtt.begin();
}

void loop() {
    mqtt.loop();

    unsigned long now = millis();
    if (now - lastRefresh >= DISPLAY_REFRESH_MS) {
        lastRefresh = now;
        display.update(data);
    }
}
