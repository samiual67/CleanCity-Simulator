// Statistics.h - reports and statistics screen
#pragma once
#include "Common.h"
#include "Simulation.h"

namespace Statistics {
    void draw(const Simulation& sim);
    std::vector<std::string> reportLines(const Simulation& sim);     // text summary of the simulation
    bool saveReport(const Simulation& sim, const char* path); bool saveReportPdf(const Simulation& sim, const char* path);        // writes report_summary.txt
    void drawReportOverlay(const Simulation& sim);
}
