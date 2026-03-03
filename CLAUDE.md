# Solar Assistant Display

Embedded project that subscribes to Solar Assistant MQTT topics and displays key inverter data on a QC12864B LCD.

## Hardware

| Component | Details |
|-----------|---------|
| MCU | Wemos D1 Mini (ESP8266) |
| Display | QC12864B (128×64 graphical LCD, serial interface) |
| Framework | Arduino via PlatformIO |
| Platform | espressif8266 |

### Pin Mapping

| Display Pin | Wemos D1 Pin | GPIO |
|-------------|--------------|------|
| RS (Chip Select / Register Select) | D8 | GPIO15 |
| RX (Data) | D7 | GPIO13 |
| E (Enable / Clock) | D5 | GPIO14 |
| VCC | 3.3V | — |
| GND | GND | — |

### Display Interface

The QC12864B uses a 3-wire serial interface compatible with ST7920-based controllers. The U8g2 library (`u8g2_rrd` or `U8G2_ST7920_128X64_F_SW_SPI`) is the recommended driver.

---

## Project Goal

Subscribe to Solar Assistant MQTT topics and render a compact dashboard on the 128×64 LCD, showing at a glance:

- **Inverter Mode** (e.g., Solar, Battery, Grid, Off)
- **Solar / PV Power** (W)
- **Battery State of Charge** (%) and Power flow (W charge/discharge)
- **Load / Inverter Output Power** (W)
- **Grid Power** (W import/export)

---

## MQTT

### Broker

Solar Assistant runs its own Mosquitto broker. Default connection:

```
Host:     <solar-assistant-ip>   (set in src/config.h)
Port:     1883
User/Pass: optional (set in src/config.h)
```

### Topics

Solar Assistant publishes state values to topics of the form:

```
solar_assistant/<device>/<metric>/state
```

Key topics used by this project:

| Topic | Description | Unit |
|-------|-------------|------|
| `solar_assistant/inverter_1/device_mode/state` | Current operating mode | string |
| `solar_assistant/inverter_1/load_power/state` | Inverter output / load | W |
| `solar_assistant/total/pv_power/state` | Total PV / Solar power | W |
| `solar_assistant/total/battery_power/state` | Battery charge (+) / discharge (−) | W |
| `solar_assistant/total/battery_state_of_charge/state` | Battery SOC | % |
| `solar_assistant/total/grid_power/state` | Grid import (+) / export (−) | W |

> Adjust topic prefixes in `src/config.h` if your Solar Assistant device ID differs (e.g., `inverter_2`).

---

## Project Structure

```
solar-assistant-display/
├── CLAUDE.md
├── platformio.ini
└── src/
    ├── main.cpp          # Setup, loop, MQTT callbacks, display refresh
    ├── config.h          # WiFi, MQTT broker, pin definitions, topic strings
    ├── display.h/.cpp    # U8g2 wrapper — layout, drawing functions
    └── mqtt_client.h/.cpp # PubSubClient wrapper — connect, subscribe, parse
```

---

## platformio.ini

```ini
[env:d1_mini]
platform  = espressif8266
board     = d1_mini
framework = arduino
monitor_speed = 115200

lib_deps =
    knolleary/PubSubClient @ ^2.8
    olikraus/U8g2 @ ^2.35
```

---

## Display Layout (128×64)

```
┌──────────────────────────────────┐
│ ██████ SOLAR+BATT █████████████ │  y  0–12  inverted bar — mode indicator
│ [▓▓▓▓▓▓▓▓▓▓▓░░░░░░░░░░░]  87%  │  y 14–22  SOC bar + percentage
│                                  │  y 23–28  gap
│ PV:  1850 W                      │  y 31     PV power
│ LOD: 1200 W                      │  y 40     Load / inverter output
│ BAT:  +650 W CHG                 │  y 49     Battery (CHG/DIS label)
│ GRD:   +50 W IMP                 │  y 58     Grid (IMP/EXP label)
└──────────────────────────────────┘
```

**Fonts:**
- Mode bar: `u8g2_font_7x13B_tf` — bold, inverted (white on black)
- Data rows: `u8g2_font_5x7_tf` — compact, fits 25 chars at 128 px

**Mode label mapping** (Solar Assistant → display):

| Solar Assistant value | Display label |
|-----------------------|---------------|
| Solar/Battery         | `SOLAR+BATT`  |
| Solar/Battery/Grid    | `SOL+BAT+GRD` |
| Battery/Grid (or AC)  | `BATT+GRID`   |
| Solar                 | `SOLAR`       |
| Battery               | `BATTERY`     |
| Grid / Line / AC      | `GRID`        |
| Fault                 | `** FAULT **` |
| Off / unknown         | `OFF`         |
| No MQTT signal (>30s) | `NO SIGNAL`   |

---

## Key Implementation Notes

- Use **PubSubClient** for MQTT; reconnect logic must be non-blocking (`millis()`-based, not `delay()`).
- Parse all incoming payloads as `float` via `atof(payload)`.
- Redraw the display only when a value changes or every 2 s max to avoid flicker.
- Store the last-received timestamp per topic; show a `?` for stale values (>30 s).
- The U8g2 `beginFullBuffer` (`_F_`) constructor is required for the ST7920 SW SPI mode on ESP8266.
- Keep `loop()` fast — no blocking calls. WiFi and MQTT reconnection must be handled with state machines.
- Battery power sign convention: **positive = charging**, **negative = discharging**. Render `CHG`/`DIS` label accordingly.
- Grid power sign convention: **positive = importing from grid**, **negative = exporting to grid**.

---

## config.h Template

```cpp
#pragma once

// WiFi
#define WIFI_SSID     "your-ssid"
#define WIFI_PASSWORD "your-password"

// MQTT Broker (Solar Assistant IP)
#define MQTT_HOST     "192.168.1.x"
#define MQTT_PORT     1883
#define MQTT_USER     ""   // leave empty if auth not enabled
#define MQTT_PASS     ""

// Display pins (Wemos D1 Mini)
#define PIN_RS  15  // D8 — CS / Register Select
#define PIN_RX  13  // D7 — Data
#define PIN_E   14  // D5 — Clock / Enable

// MQTT topics
#define TOPIC_MODE      "solar_assistant/inverter_1/device_mode/state"
#define TOPIC_LOAD      "solar_assistant/total/load_power/state"
#define TOPIC_PV        "solar_assistant/total/pv_power/state"
#define TOPIC_BATT_W    "solar_assistant/total/battery_power/state"
#define TOPIC_BATT_SOC  "solar_assistant/total/battery_state_of_charge/state"
#define TOPIC_GRID      "solar_assistant/total/grid_power/state"

// Refresh / stale thresholds
#define DISPLAY_REFRESH_MS  2000
#define STALE_THRESHOLD_MS  30000
```

---

## Development Workflow

```bash
# Build and upload
pio run -t upload

# Monitor serial output
pio device monitor

# Build only
pio run
```

---

## Dependencies

| Library | Version | Purpose |
|---------|---------|---------|
| PubSubClient | ^2.8 | MQTT client |
| U8g2 | ^2.35 | ST7920 display driver |
