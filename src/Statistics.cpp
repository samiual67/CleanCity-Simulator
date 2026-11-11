#include "Statistics.h"
#include "UIComponents.h"

static std::string num(float v) { char b[32]; std::snprintf(b, sizeof(b), "%.0f", v); return b; }

namespace Statistics {

void draw(const Simulation& sim) {
    const WasteManager& w = sim.waste;
    float t = sim.time;
    float remaining = w.totalRemaining();
    float rate = (w.totalCollected + remaining) > 0.01f ? 100.0f * w.totalCollected / (w.totalCollected + remaining) : 0;

    Gfx::text(24, 72, "Reports & Statistics", C_TEXT, FONT_M);
    float cw = 300, ch = 92, gap = 16, y0 = 82;
    UI::statCard(24, y0, cw, ch, "TOTAL COLLECTED", num(w.totalCollected) + " kg", "since simulation start", C_TEAL, t);
    UI::statCard(24 + (cw + gap), y0, cw, ch, "TOTAL REMAINING", num(remaining) + " kg", "currently in bins", C_ORANGE, t);
    UI::statCard(24 + 2 * (cw + gap), y0, cw, ch, "COMPLETION RATE", num(rate) + "%", "collected / (collected + remaining)", C_GREEN, t);
    UI::statCard(24 + 3 * (cw + gap), y0, cw, ch, "COLLECTIONS", num((float)w.collectionsDone), "bins emptied", C_BLUE, t);

    // category bar chart
    float py = y0 + ch + 14;
    UI::panel(24, py, 620, 270, "Waste by Category (kg)");
    std::vector<std::string> lab; std::vector<float> v; std::vector<Color> c;
    float mx = 10;
    for (int i = 0; i < W_COUNT; i++) {
        lab.push_back(std::string(wasteInfo(i).name) + " C");
        v.push_back(w.collected[i]); c.push_back(wasteInfo(i).color);
        lab.push_back(std::string(wasteInfo(i).name) + " R");
        v.push_back(w.remainingByCategory(i)); c.push_back(withAlpha(brighten(wasteInfo(i).color, 0.55f), 1));
    }
    for (size_t i = 0; i < v.size(); i++) mx = std::max(mx, v[i]);
    // short labels (first letters) so they fit under the bars
    for (size_t i = 0; i < lab.size(); i++) lab[i] = lab[i].substr(0, 3) + (i % 2 ? " R" : " C");
    UI::barChart(70, py + 50, 556, 170, lab, v, c, std::ceil(mx / 50.0f) * 50.0f);
    Gfx::text(70, py + 262, "C = collected    R = remaining in bins", C_DIM);

    // zone cleanliness
    UI::panel(660, py, 596, 270, "Zone Cleanliness");
    for (int z = 0; z < WasteManager::ZONES; z++) {
        ZoneStat zs = w.zone(z);
        float zy = py + 58 + z * 52;
        Gfx::text(678, zy, zs.name, C_TEXT);
        Gfx::text(678, zy + 16, std::to_string(zs.bins) + " bins, " + num(zs.waste) + " kg waste" +
                  (zs.collectionPending ? ", truck active" : ""), C_DIM);
        Gfx::textRight(1236, zy, num(zs.cleanliness) + "%  " + zs.status, zs.color);
        UI::progressBar(940, zy + 8, 296, 10, zs.cleanliness / 100.0f, zs.color);
    }

    // truck collection history
    float by = py + 284, bh = WIN_H - by - 12;
    UI::panel(24, by, 620, bh, "Truck Collection History");
    Gfx::text(40, by + 52, "TIME", C_DIM); Gfx::text(120, by + 52, "TRUCK", C_DIM);
    Gfx::text(210, by + 52, "DUSTBIN", C_DIM); Gfx::text(310, by + 52, "CATEGORY", C_DIM); Gfx::text(450, by + 52, "AMOUNT", C_DIM);
    int rows = (int)((bh - 70) / 20), shown = 0;
    if (w.history.empty()) Gfx::text(40, by + 80, "No collections yet - press Start, then Dispatch Garbage Truck.", C_DIM);
    for (int i = (int)w.history.size() - 1; i >= 0 && shown < rows; i--, shown++) {
        const CollectionRecord& r = w.history[i];
        float ry = by + 74 + shown * 20;
        char tb[16]; std::snprintf(tb, sizeof(tb), "%02d:%02d", (int)r.time / 60, (int)r.time % 60);
        Gfx::text(40, ry, tb, C_TEXT);
        Gfx::text(120, ry, r.truckId >= 0 ? "TR-" + std::to_string(r.truckId + 1) : std::string("Manual"), C_TEXT);
        Gfx::text(210, ry, "DB-" + std::string(r.binId + 1 < 10 ? "0" : "") + std::to_string(r.binId + 1), C_TEXT);
        Gfx::circle(318, ry - 4, 4, wasteInfo(r.category).color, 8);
        Gfx::text(328, ry, wasteInfo(r.category).name, C_TEXT);
        Gfx::text(450, ry, num(r.amount) + " kg", C_GREEN);
    }

    // per truck summary + report button area
    UI::panel(660, by, 596, bh, "Fleet Summary");
    for (size_t i = 0; i < sim.trucks.size(); i++) {
        const GarbageTruck& tr = sim.trucks[i];
        float ty = by + 58 + i * 40;
        Gfx::roundRect(678, ty - 14, 14, 14, 4, tr.color);
        Gfx::text(700, ty, tr.label() + "  -  " + tr.statusText(), C_TEXT);
        Gfx::text(700, ty + 16, std::to_string(tr.collections) + " collections, " + num(tr.totalCollected) + " kg collected", C_DIM);
    }
}

std::vector<std::string> reportLines(const Simulation& sim) {
    const WasteManager& w = sim.waste;
    std::vector<std::string> L;
    char b[160];
    int sec = (int)w.clock;
    std::snprintf(b, sizeof(b), "Simulation time: %02d:%02d   Status: %s", sec / 60, sec % 60,
                  !sim.started ? "not started" : (sim.paused ? "paused" : "running"));
    L.push_back(b);
    std::snprintf(b, sizeof(b), "Dustbins: %d total, %d full, %d partial, %d empty", (int)w.bins.size(), w.countFull(), w.countPartial(), w.countEmpty());
    L.push_back(b);
    std::snprintf(b, sizeof(b), "Waste collected: %.0f kg in %d collections", w.totalCollected, w.collectionsDone);
    L.push_back(b);
    std::snprintf(b, sizeof(b), "Waste remaining in bins: %.0f kg", w.totalRemaining());
    L.push_back(b);
    std::snprintf(b, sizeof(b), "City cleanliness: %.0f%%", w.cityCleanliness());
    L.push_back(b);
    L.push_back("");
    L.push_back("Waste by category (collected / remaining / recycled):");
    for (int c = 0; c < W_COUNT; c++) {
        std::snprintf(b, sizeof(b), "  %-8s %6.0f kg  /  %6.0f kg  /  %6.0f kg", wasteInfo(c).name, w.collected[c],
                      w.remainingByCategory(c), sim.recycling.processed[c]);
        L.push_back(b);
    }
    L.push_back("");
    L.push_back("Zone cleanliness:");
    for (int z = 0; z < WasteManager::ZONES; z++) {
        ZoneStat zs = w.zone(z);
        std::snprintf(b, sizeof(b), "  %-15s %3.0f%%  (%s)", zs.name.c_str(), zs.cleanliness, zs.status);
        L.push_back(b);
    }
    L.push_back("");
    L.push_back("Trucks:");
    for (size_t i = 0; i < sim.trucks.size(); i++) {
        std::snprintf(b, sizeof(b), "  %s: %d collections, %.0f kg - %s", sim.trucks[i].label().c_str(),
                      sim.trucks[i].collections, sim.trucks[i].totalCollected, sim.trucks[i].statusText().c_str());
        L.push_back(b);
    }
    return L;
}

bool saveReport(const Simulation& sim, const char* path) {
    FILE* f = std::fopen(path, "w");
    if (!f) return false;
    std::fprintf(f, "CleanCity Simulator - Simulation Report\n=================================\n");
    std::vector<std::string> L = reportLines(sim);
    for (size_t i = 0; i < L.size(); i++) std::fprintf(f, "%s\n", L[i].c_str());
    std::fclose(f);
    return true;
}

void drawReportOverlay(const Simulation& sim) {
    Gfx::rect(0, 0, WIN_W, WIN_H, Color(0, 0, 0, 0.65f));
    float w = 700, h = 520, x = (WIN_W - w) / 2, y = (WIN_H - h) / 2;
    UI::panel(x, y, w, h, "Simulation Report");
    std::vector<std::string> L = reportLines(sim);
    for (size_t i = 0; i < L.size(); i++) Gfx::text(x + 24, y + 56 + i * 20, L[i], C_TEXT);
    Gfx::text(x + 24, y + h - 18, "Saved to report_summary.pdf  -  click anywhere or press Esc to close", C_GREEN);
}

} // namespace Statistics
