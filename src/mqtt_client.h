#pragma once

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "config.h"
#include "display.h"

class MqttClient {
public:
    explicit MqttClient(SolarData& data);
    void begin();
    void loop();
    bool connected() { return _client.connected(); }

private:
    SolarData&    _data;
    WiFiClient    _wifi;
    PubSubClient  _client;
    unsigned long _lastReconnect;

    void reconnect();
    void subscribe();
    void handleMessage(const char* topic, const char* value);

    // PubSubClient requires a plain C callback; static trampoline → instance
    static MqttClient* _instance;
    static void onMessage(char* topic, byte* payload, unsigned int len);
};
