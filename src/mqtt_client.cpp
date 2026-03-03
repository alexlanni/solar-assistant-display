#include "mqtt_client.h"
#include <string.h>

MqttClient* MqttClient::_instance = nullptr;

MqttClient::MqttClient(SolarData& data)
    : _data(data), _client(_wifi), _lastReconnect(0) {
    _instance = this;
}

void MqttClient::begin() {
    _lastReconnect = millis() - MQTT_RECONNECT_MS;  // triggers immediate first attempt
    _client.setServer(MQTT_HOST, MQTT_PORT);
    _client.setCallback(onMessage);
}

void MqttClient::subscribe() {
    _client.subscribe(TOPIC_MODE);
    _client.subscribe(TOPIC_PV);
    _client.subscribe(TOPIC_LOAD);
    _client.subscribe(TOPIC_BATT_W);
    _client.subscribe(TOPIC_BATT_SOC);
    _client.subscribe(TOPIC_GRID);
    _client.subscribe(TOPIC_PV_ENERGY);
    _client.subscribe(TOPIC_LOAD_ENERGY);
    _client.subscribe(TOPIC_GRID_ENERGY_IN);
}

void MqttClient::reconnect() {
    unsigned long now = millis();
    if (now - _lastReconnect < MQTT_RECONNECT_MS) return;
    _lastReconnect = now;

    const char* user = strlen(MQTT_USER) ? MQTT_USER : nullptr;
    const char* pass = strlen(MQTT_PASS) ? MQTT_PASS : nullptr;

    Serial.print("MQTT connecting...");
    if (_client.connect(MQTT_CLIENT_ID, user, pass)) {
        Serial.println(" OK");
        subscribe();
    } else {
        Serial.print(" failed, rc=");
        Serial.println(_client.state());
    }
}

void MqttClient::loop() {
    if (!_client.connected()) {
        reconnect();
    } else {
        _client.loop();
    }
}

// Static trampoline: null-terminates payload and delegates to instance.
void MqttClient::onMessage(char* topic, byte* payload, unsigned int len) {
    if (!_instance) return;
    char value[64] = {0};
    if (len >= sizeof(value)) len = sizeof(value) - 1;
    memcpy(value, payload, len);
    _instance->handleMessage(topic, value);
}

void MqttClient::handleMessage(const char* topic, const char* value) {
    unsigned long now = millis();

    if (strcmp(topic, TOPIC_MODE) == 0) {
        strncpy(_data.mode, value, sizeof(_data.mode) - 1);
        _data.mode[sizeof(_data.mode) - 1] = '\0';
        _data.modeTs = now;
    } else if (strcmp(topic, TOPIC_PV) == 0) {
        _data.pvPower = atof(value);
        _data.pvTs    = now;
    } else if (strcmp(topic, TOPIC_LOAD) == 0) {
        _data.loadPower = atof(value);
        _data.loadTs    = now;
    } else if (strcmp(topic, TOPIC_BATT_W) == 0) {
        _data.battPower   = atof(value);
        _data.battPowerTs = now;
    } else if (strcmp(topic, TOPIC_BATT_SOC) == 0) {
        _data.battSoc   = atof(value);
        _data.battSocTs = now;
    } else if (strcmp(topic, TOPIC_GRID) == 0) {
        _data.gridPower = atof(value);
        _data.gridTs    = now;
    } else if (strcmp(topic, TOPIC_PV_ENERGY) == 0) {
        _data.pvEnergy   = atof(value);
        _data.pvEnergyTs = now;
    } else if (strcmp(topic, TOPIC_LOAD_ENERGY) == 0) {
        _data.loadEnergy   = atof(value);
        _data.loadEnergyTs = now;
    } else if (strcmp(topic, TOPIC_GRID_ENERGY_IN) == 0) {
        _data.gridEnergyIn   = atof(value);
        _data.gridEnergyInTs = now;
    }
}
