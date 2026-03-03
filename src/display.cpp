#include "display.h"
#include <Arduino.h>

// ST7920 SW SPI: U8G2(rotation, clock=E, data=RX, cs=RS)
Display::Display()
    : _u8g2(U8G2_R0, PIN_E, PIN_RX, PIN_RS) {}

void Display::begin() {
    _u8g2.begin();
}

// Simple two-line status screen used during boot / reconnect.
void Display::showStatus(const char* line1, const char* line2) {
    _u8g2.clearBuffer();
    _u8g2.setFont(u8g2_font_7x13B_tf);
    uint8_t tw = _u8g2.getStrWidth(line1);
    _u8g2.drawStr((128 - tw) / 2, 28, line1);
    if (line2) {
        _u8g2.setFont(u8g2_font_5x7_tf);
        tw = _u8g2.getStrWidth(line2);
        _u8g2.drawStr((128 - tw) / 2, 42, line2);
    }
    _u8g2.sendBuffer();
}

bool Display::isStale(unsigned long ts) {
    return (ts == 0) || (millis() - ts > STALE_THRESHOLD_MS);
}

// Maps Solar Assistant mode strings to compact display labels.
const char* Display::modeLabel(const char* mode) {
    bool hasSolar  = strstr(mode, "Solar")   != nullptr;
    bool hasBatt   = strstr(mode, "Battery") != nullptr;
    bool hasGrid   = strstr(mode, "Grid")    != nullptr
                  || strstr(mode, "Line")    != nullptr
                  || strstr(mode, "AC")      != nullptr;

    if (hasSolar && hasBatt && hasGrid) return "SOL+BAT+GRD";
    if (hasSolar && hasBatt)            return "SOLAR+BATT";
    if (hasBatt  && hasGrid)            return "BATT+GRID";
    if (hasSolar)                       return "SOLAR";
    if (hasBatt)                        return "BATTERY";
    if (hasGrid)                        return "GRID";
    if (strstr(mode, "Fault"))          return "** FAULT **";
    if (strstr(mode, "Off") || strlen(mode) == 0) return "OFF";
    return mode;   // fallback: show raw string
}

// Row 0–12: inverted rectangle with centered mode text.
void Display::drawMode(const SolarData& data) {
    const char* label = isStale(data.modeTs) ? "NO SIGNAL" : modeLabel(data.mode);

    _u8g2.setDrawColor(1);
    _u8g2.drawBox(0, 0, 128, 13);         // filled background

    _u8g2.setDrawColor(0);                 // white text on black box
    _u8g2.setFont(u8g2_font_7x13B_tf);
    uint8_t tw = _u8g2.getStrWidth(label);
    _u8g2.drawStr((128 - tw) / 2, 11, label);

    _u8g2.setDrawColor(1);                 // restore
}

// Row 14–22: SOC progress bar + percentage.
void Display::drawSocBar(float soc, bool stale) {
    _u8g2.setFont(u8g2_font_5x7_tf);

    const uint8_t barX = 0, barY = 14, barW = 96, barH = 8;
    _u8g2.drawFrame(barX, barY, barW, barH);

    if (!stale) {
        uint8_t fill = (uint8_t)((barW - 2) * soc / 100.0f);
        if (fill > barW - 2) fill = barW - 2;
        if (fill > 0) _u8g2.drawBox(barX + 1, barY + 1, fill, barH - 2);
    }

    char buf[8];
    if (stale) snprintf(buf, sizeof(buf), "  ?%%");
    else        snprintf(buf, sizeof(buf), "%3d%%", (int)soc);
    _u8g2.drawStr(barX + barW + 2, barY + 7, buf);
}

// Dual power+energy row — left: label+power at x=0, right: energy right-aligned.
void Display::drawDualPowerRow(uint8_t y, const char* label, float watts, float kwh, bool stalePwr, bool staleEne) {
    _u8g2.setFont(u8g2_font_5x7_tf);
    char left[14], right[10];
    if (stalePwr) snprintf(left,  sizeof(left),  "%-4s   ? W", label);
    else          snprintf(left,  sizeof(left),  "%-4s%4d W", label, (int)watts);
    if (staleEne) strcpy(right, "?kWh");
    else          snprintf(right, sizeof(right), "%.1fkWh", kwh);
    _u8g2.drawStr(0, y, left);
    _u8g2.drawStr(128 - _u8g2.getStrWidth(right), y, right);
}

// Battery row — left: power at x=0, right: CHG/DIS right-aligned.
void Display::drawBattRow(uint8_t y, float watts, bool stale) {
    _u8g2.setFont(u8g2_font_5x7_tf);
    if (stale) {
        _u8g2.drawStr(0, y, "BAT:   ? W");
        return;
    }
    char left[12];
    snprintf(left, sizeof(left), "BAT:%4d W", (int)fabsf(watts));
    const char* dir = (watts >= 0) ? "CHG" : "DIS";
    _u8g2.drawStr(0, y, left);
    _u8g2.drawStr(128 - _u8g2.getStrWidth(dir), y, dir);
}

// Grid row — left: power+direction at x=0, right: energy right-aligned.
void Display::drawGridRow(uint8_t y, float watts, float kwh, bool stalePwr, bool staleEne) {
    _u8g2.setFont(u8g2_font_5x7_tf);
    char left[16], right[10];
    if (stalePwr) {
        strcpy(left, "GRD    0 W");
    } else {
        const char* dir = (watts > 10.0f) ? "IMP" : (watts < -10.0f) ? "EXP" : nullptr;
        if (dir) snprintf(left, sizeof(left), "GRD %4d W %s", (int)fabsf(watts), dir);
        else     snprintf(left, sizeof(left), "GRD %4d W",    (int)fabsf(watts));
    }
    if (staleEne) strcpy(right, "?kWh");
    else          snprintf(right, sizeof(right), "%.1fkWh", kwh);
    _u8g2.drawStr(0, y, left);
    _u8g2.drawStr(128 - _u8g2.getStrWidth(right), y, right);
}

void Display::update(const SolarData& data) {
    _u8g2.clearBuffer();

    drawMode(data);                                                      // y  0–12
    drawSocBar(data.battSoc, isStale(data.battSocTs));                  // y 14–22
    drawDualPowerRow(31, "PV:", data.pvPower, data.pvEnergy,
                     isStale(data.pvTs), data.pvEnergyTs == 0);         // y 31
    drawDualPowerRow(41, "LOD:", data.loadPower, data.loadEnergy,
                     isStale(data.loadTs), data.loadEnergyTs == 0);     // y 41
    drawBattRow(51, data.battPower, isStale(data.battPowerTs));         // y 51
    drawGridRow(61, data.gridPower, data.gridEnergyIn,
                isStale(data.gridTs), data.gridEnergyInTs == 0);        // y 61

    _u8g2.sendBuffer();
}
