#pragma once

// ── WiFi ──────────────────────────────────────────────────────────────────────
#define WIFI_SSID     ""
#define WIFI_PASSWORD ""

// ── MQTT Broker (Solar Assistant IP) ─────────────────────────────────────────
#define MQTT_HOST      "192.168.1.x"
#define MQTT_PORT      1883
#define MQTT_USER      ""          // leave empty if auth not enabled
#define MQTT_PASS      ""
#define MQTT_CLIENT_ID "solar-display"

// ── Display pins (Wemos D1 Mini) ─────────────────────────────────────────────
#define PIN_RS  15   // D8 — CS / Register Select
#define PIN_RX  13   // D7 — Data
#define PIN_E   14   // D5 — Clock / Enable

// ── MQTT topics ───────────────────────────────────────────────────────────────
#define TOPIC_MODE     "solar_assistant/inverter_1/device_mode/state"
#define TOPIC_PV       "solar_assistant/inverter_1/pv_power/state"
#define TOPIC_LOAD     "solar_assistant/inverter_1/load_power/state"
#define TOPIC_BATT_W   "solar_assistant/total/battery_power/state"
#define TOPIC_BATT_SOC "solar_assistant/total/battery_state_of_charge/state"
#define TOPIC_GRID     "ssolar_assistant/inverter_1/grid_power/state"
#define TOPIC_PV_ENERGY    "solar_assistant/total/pv_energy/state"
#define TOPIC_LOAD_ENERGY  "solar_assistant/total/load_energy/state"
#define TOPIC_GRID_ENERGY_IN "solar_assistant/total/grid_energy_in/state"

// ── Timing ───────────────────────────────────────────────────────────────────
#define DISPLAY_REFRESH_MS  2000
#define STALE_THRESHOLD_MS  30000
#define MQTT_RECONNECT_MS   5000
