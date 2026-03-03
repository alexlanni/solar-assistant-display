# Solar Assistant Display

Embedded dashboard that subscribes to [Solar Assistant](https://solar-assistant.io) MQTT topics and renders live inverter data on a 128×64 graphical LCD.

## Hardware

| Component | Details |
|-----------|---------|
| MCU | Wemos D1 Mini (ESP8266) |
| Display | QC12864B — 128×64 graphical LCD, ST7920 controller |
| Interface | 3-wire serial (SW SPI) |
| Framework | Arduino via PlatformIO |

### Wiring

| Display pin | Wemos D1 pin | GPIO |
|-------------|--------------|------|
| RS (CS) | D8 | 15 |
| RX (Data) | D7 | 13 |
| E (Clock) | D5 | 14 |
| VCC | 3.3 V | — |
| GND | GND | — |

## Project Structure

```
solar-assistant-display/
├── platformio.ini
└── src/
    ├── config.h          # WiFi, MQTT broker, pins, topic strings
    ├── main.cpp          # setup(), loop(), display refresh
    ├── display.h/.cpp    # U8g2 driver — layout and drawing
    └── mqtt_client.h/.cpp # PubSubClient — connect, subscribe, parse
```

## Dependencies

| Library | Version | Purpose |
|---------|---------|---------|
| [PubSubClient](https://github.com/knolleary/pubsubclient) | ^2.8 | MQTT client |
| [U8g2](https://github.com/olikraus/u8g2) | ^2.35 | ST7920 display driver |

## Configuration

Copy the template below into `src/config.h` and fill in your values:

```cpp
#pragma once

// WiFi
#define WIFI_SSID     "your-ssid"
#define WIFI_PASSWORD "your-password"

// MQTT Broker (Solar Assistant IP)
#define MQTT_HOST      "192.168.1.x"
#define MQTT_PORT      1883
#define MQTT_USER      ""   // leave empty if auth is not enabled
#define MQTT_PASS      ""
#define MQTT_CLIENT_ID "solar-display"

// Display pins (Wemos D1 Mini)
#define PIN_RS  15   // D8 — CS / Register Select
#define PIN_RX  13   // D7 — Data
#define PIN_E   14   // D5 — Clock / Enable

// MQTT topics
#define TOPIC_MODE           "solar_assistant/inverter_1/device_mode/state"
#define TOPIC_PV             "solar_assistant/inverter_1/pv_power/state"
#define TOPIC_LOAD           "solar_assistant/inverter_1/load_power/state"
#define TOPIC_BATT_W         "solar_assistant/total/battery_power/state"
#define TOPIC_BATT_SOC       "solar_assistant/total/battery_state_of_charge/state"
#define TOPIC_GRID           "solar_assistant/inverter_1/grid_power/state"
#define TOPIC_PV_ENERGY      "solar_assistant/total/pv_energy/state"
#define TOPIC_LOAD_ENERGY    "solar_assistant/total/load_energy/state"
#define TOPIC_GRID_ENERGY_IN "solar_assistant/total/grid_energy_in/state"

// Timing
#define DISPLAY_REFRESH_MS  2000
#define STALE_THRESHOLD_MS  30000
#define MQTT_RECONNECT_MS   5000
```

> Adjust `inverter_1` to match your Solar Assistant device ID if needed.

## Display Layout

```
┌────────────────────────────────┐
│      ██ SOLAR+BATT ████████   │  mode bar (inverted, white on black)
│ [▓▓▓▓▓▓▓▓▓▓░░░░░░░░]   87%   │  battery SOC bar + percentage
│                                │
│ PV:  1850 W          12.3kWh  │  PV power | daily PV yield
│ LOD: 1200 W           9.1kWh  │  load power | daily load energy
│ BAT:  650 W               CHG │  battery power | charge/discharge
│ GRD:   50 W IMP        0.2kWh │  grid power + direction | daily import
└────────────────────────────────┘
```

Each row uses the full 128 px width: the left value is drawn from x=0, the right value is pixel-perfect right-aligned.

### Mode labels

| Solar Assistant value | Display |
|-----------------------|---------|
| Solar/Battery | `SOLAR+BATT` |
| Solar/Battery/Grid | `SOL+BAT+GRD` |
| Battery/Grid (or AC/Line) | `BATT+GRID` |
| Solar | `SOLAR` |
| Battery | `BATTERY` |
| Grid / Line / AC | `GRID` |
| Fault | `** FAULT **` |
| Off / unknown | `OFF` |
| No MQTT signal (>30 s) | `NO SIGNAL` |

### Sign conventions

| Value | Positive | Negative |
|-------|----------|----------|
| Battery power | Charging | Discharging |
| Grid power | Importing | Exporting |

### Stale values

- Power values (PV, load, battery, grid): show `?` if no MQTT message received in the last 30 s.
- Grid power stale: shows `0 W` (grid defaults to zero when offline).
- Energy values (PV yield, load energy, grid import): retain the last received value indefinitely — they are daily accumulators that reset at midnight and do not publish continuously.

## Build & Flash

```bash
# Build and upload
pio run -t upload

# Monitor serial output (115200 baud)
pio device monitor

# Build only
pio run
```

## Boot Sequence

1. LCD initialises and shows `Connecting WiFi`
2. After WiFi connects: shows IP address for 600 ms
3. Shows `MQTT connecting` while establishing broker connection
4. Dashboard appears as soon as the first MQTT messages arrive
5. Non-blocking reconnect retries every 5 s if the broker drops
