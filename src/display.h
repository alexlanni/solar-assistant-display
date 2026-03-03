#pragma once

#include <U8g2lib.h>
#include "config.h"

struct SolarData {
    char          mode[32];
    float         pvPower;
    float         loadPower;
    float         battPower;
    float         battSoc;
    float         gridPower;
    float         pvEnergy;
    float         loadEnergy;
    float         gridEnergyIn;
    unsigned long modeTs;
    unsigned long pvTs;
    unsigned long loadTs;
    unsigned long battPowerTs;
    unsigned long battSocTs;
    unsigned long gridTs;
    unsigned long pvEnergyTs;
    unsigned long loadEnergyTs;
    unsigned long gridEnergyInTs;
};

class Display {
public:
    Display();
    void begin();
    void showStatus(const char* line1, const char* line2 = nullptr);
    void update(const SolarData& data);

private:
    U8G2_ST7920_128X64_F_SW_SPI _u8g2;

    void drawMode(const SolarData& data);
    void drawSocBar(float soc, bool stale);
    void drawDualPowerRow(uint8_t y, const char* label, float watts, float kwh, bool stalePwr, bool staleEne);
    void drawBattRow(uint8_t y, float watts, bool stale);
    void drawGridRow(uint8_t y, float watts, float kwh, bool stalePwr, bool staleEne);

    bool        isStale(unsigned long ts);
    const char* modeLabel(const char* mode);
};
