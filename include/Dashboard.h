// Dashboard.h - main dashboard, recycling centre screen (live statistic views)
#pragma once
#include "Common.h"
#include "Animation.h"
#include "Simulation.h"

class Dashboard {
public:
    Dashboard();
    void update(float dt, const Simulation& sim);     // animated count-up of the numbers
    void draw(const Simulation& sim);                 // dashboard screen
    void drawRecycling(const Simulation& sim);        // recycling center screen
private:
    SmoothValue total, full, partial, empty, collected, done, clean, active;
    SmoothValue cleanGauge;
    SmoothValue zoneBar[WasteManager::ZONES];
};
