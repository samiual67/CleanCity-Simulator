#include "Dashboard.h"
#include "UIComponents.h"

static std::string num(float v, int decimals = 0) {
    char b[32];
    std::snprintf(b, sizeof(b), decimals ? "%.1f" : "%.0f", v);
    return b;
}

Dashboard::Dashboard() {}

void Dashboard::update(float dt, const Simulation& sim) {
    const WasteManager& w = sim.waste;
    total.target = (float)w.bins.size();
    full.target = (float)w.countFull();
    partial.target = (float)w.countPartial();
    empty.target = (float)w.countEmpty();
    collected.target = w.totalCollected;
    done.target = (float)w.collectionsDone;
    clean.target = w.cityCleanliness();
    active.target = (float)sim.activeTrucks();
    cleanGauge.target = w.cityCleanliness() / 100.0f;
    for (int z = 0; z < WasteManager::ZONES; z++) zoneBar[z].target = w.zone(z).cleanliness;
    total.update(dt); full.update(dt); partial.update(dt); empty.update(dt);
    collected.update(dt); done.update(dt); clean.update(dt); active.update(dt);
    cleanGauge.update(dt, 4); 
    for (int z = 0; z < WasteManager::ZONES; z++) zoneBar[z].update(dt);
}

void Dashboard::draw(const Simulation& sim) {
    const WasteManager& w = sim.waste;
    float t = sim.time;
    float cw = 300, ch = 98, gap = 16, x0 = 24, y0 = 72;

    Gfx::text(x0, y0 - 2, "City Overview", C_TEXT, FONT_M);
    float cleanPct = w.cityCleanliness();
    Color cc = cleanPct >= 60 ? C_GREEN : (cleanPct >= 35 ? C_YELLOW : C_RED);
    y0 += 12;

    UI::statCard(x0 + 0 * (cw + gap), y0, cw, ch, "TOTAL DUSTBINS", num(total.cur), "smart bins online", C_BLUE, t);
    UI::statCard(x0 + 1 * (cw + gap), y0, cw, ch, "FULL DUSTBINS", num(full.cur), "need urgent pickup", C_RED, t);
    UI::statCard(x0 + 2 * (cw + gap), y0, cw, ch, "PARTIALLY FILLED", num(partial.cur), "between 5% and 99%", C_ORANGE, t);
    UI::statCard(x0 + 3 * (cw + gap), y0, cw, ch, "EMPTY DUSTBINS", num(empty.cur), "below 5% filled", C_GREEN, t);
    float y1 = y0 + ch + gap;
    UI::statCard(x0 + 0 * (cw + gap), y1, cw, ch, "WASTE COLLECTED", num(collected.cur) + " kg", "total by all trucks", C_TEAL, t);
    UI::statCard(x0 + 1 * (cw + gap), y1, cw, ch, "COMPLETED COLLECTIONS", num(done.cur), "bins emptied", C_BLUE, t);
    UI::statCard(x0 + 2 * (cw + gap), y1, cw, ch, "CITY CLEANLINESS", num(clean.cur) + "%",
                 cleanPct >= 60 ? "city is clean" : (cleanPct >= 35 ? "needs attention" : "critical!"), cc, t);
    UI::statCard(x0 + 3 * (cw + gap), y1, cw, ch, "ACTIVE TRUCKS", num(active.cur) + " / " + num((float)sim.trucks.size()),
                 sim.running() ? "simulation running" : "simulation idle", C_YELLOW, t);

    float py = y1 + ch + gap + 4, ph = WIN_H - py - 12;

    // --- waste distribution chart
    UI::panel(24, py, 400, ph, "Waste Distribution (in bins)");
    std::vector<float> vals; std::vector<Color> cols;
    for (int c = 0; c < W_COUNT; c++) { vals.push_back(w.remainingByCategory(c)); cols.push_back(wasteInfo(c).color); }
    UI::pieChart(24 + 120, py + ph / 2 + 4, 86, vals, cols);
    float tot = w.totalRemaining();
    Gfx::textCentered(24 + 120, py + ph / 2 + 2, num(tot) + " kg", C_TEXT, FONT_S);
    Gfx::textCentered(24 + 120, py + ph / 2 + 18, "remaining", C_DIM, FONT_S);
    for (int c = 0; c < W_COUNT; c++) {
        float ly = py + 66 + c * 34;
        Gfx::roundRect(24 + 250, ly - 10, 14, 14, 4, cols[c]);
        Gfx::text(24 + 272, ly + 2, wasteInfo(c).name, C_TEXT);
        float pct = tot > 0.01f ? 100 * vals[c] / tot : 0;
        Gfx::text(24 + 272, ly + 17, num(vals[c]) + " kg  (" + num(pct) + "%)", C_DIM);
    }

    // --- cleanliness indicator + zones
    UI::panel(440, py, 410, ph, "Cleanliness Status");
    UI::gauge(440 + 100, py + 125, 70, cleanGauge.cur, cc);
    Gfx::text(440 + 190, py + 92, cleanPct >= 60 ? "CLEAN" : (cleanPct >= 35 ? "NEEDS ATTENTION" : "CRITICAL"), cc, FONT_M);
    Gfx::text(440 + 190, py + 114, "Overall city score", C_DIM);
    Gfx::text(440 + 190, py + 134, "Updates when waste", C_DIM);
    Gfx::text(440 + 190, py + 149, "changes or is collected", C_DIM);
    for (int z = 0; z < WasteManager::ZONES; z++) {
        ZoneStat zs = w.zone(z);
        float zy = py + 200 + z * 44;
        Gfx::text(456, zy, zs.name, C_TEXT);
        Gfx::textRight(836, zy, num(zoneBar[z].cur) + "%  " + zs.status, zs.color);
        UI::progressBar(456, zy + 8, 380, 9, zoneBar[z].cur / 100.0f, zs.color);
    }

    // --- recent activity
    UI::panel(866, py, 390, ph, "Recent Activity");
    int shown = 0;
    for (int i = (int)w.log.size() - 1; i >= 0 && shown < (int)((ph - 50) / 20); i--, shown++) {
        float ly = py + 52 + shown * 20;
        float a = 1.0f - shown * 0.07f;
        Gfx::circle(880, ly - 4, 3, withAlpha(C_GREEN, a), 8);
        std::string s = w.log[i];
        while (Gfx::textWidth(s) > 350 && s.size() > 4) s.erase(s.size() - 1);
        Gfx::text(892, ly, s, withAlpha(C_TEXT, a));
    }
}

void Dashboard::drawRecycling(const Simulation& sim) {
    const WasteManager& w = sim.waste;
    const RecyclingCenter& rc = sim.recycling;
    float t = sim.time;
    Gfx::text(24, 72, "Recycling Center", C_TEXT, FONT_M);

    UI::panel(24, 84, 790, 628, "Waste Categories");
    // table header
    float hx = 150;
    Gfx::text(hx + 20, 126, "COLLECTED", C_DIM);
    Gfx::text(hx + 120, 126, "DELIVERED", C_DIM);
    Gfx::text(hx + 220, 126, "RECYCLABLE", C_DIM);
    Gfx::text(hx + 330, 126, "RECYCLING PROGRESS", C_DIM);
    for (int c = 0; c < W_COUNT; c++) {
        float ry = 138 + c * 114;
        Color col = wasteInfo(c).color;
        Gfx::roundRect(38, ry, 762, 104, 10, Color(0, 0, 0, 0.22f));
        Gfx::roundRect(38, ry + 8, 5, 88, 2, col);
        UI::wasteIcon(88, ry + 44, 54, c);
        Gfx::text(130, ry + 22, wasteInfo(c).name, C_TEXT, FONT_M);
        Gfx::text(hx - 20 + 40, ry + 56, num(w.collected[c]) + " kg", C_TEXT);
        Gfx::text(hx + 120, ry + 56, num(rc.received[c]) + " kg", C_TEXT);
        Gfx::text(hx + 220, ry + 56, num(rc.recyclable(c)) + " kg", col);
        float prog = rc.progress(c);
        UI::progressBar(hx + 330, ry + 44, 270, 14, prog, col);
        Gfx::text(hx + 330, ry + 76, "Processed " + num(rc.processed[c]) + " kg  (" + num(prog * 100) + "%)", C_DIM);
        Gfx::text(hx + 330, ry + 92, "Recycle rate " + num(wasteInfo(c).recycleRate * 100) + "%", C_DIM);
        // info text, trimmed to fit
        std::string info = wasteInfo(c).info;
        while (Gfx::textWidth(info) > 300 && info.size() > 4) info.erase(info.size() - 1);
        Gfx::text(130, ry + 78, info, C_DIM);
        // small progress dots under the title
        Gfx::text(130, ry + 94, "Rate: " + num(wasteInfo(c).recycleRate * 100) + "% recyclable", withAlpha(col, 0.9f));
    }

    // right column
    UI::panel(830, 84, 426, 300, "Collected by Category");
    std::vector<float> vals; std::vector<Color> cols;
    float sum = 0;
    for (int c = 0; c < W_COUNT; c++) { vals.push_back(w.collected[c]); cols.push_back(wasteInfo(c).color); sum += w.collected[c]; }
    UI::pieChart(930, 245, 80, vals, cols);
    if (sum < 0.01f) Gfx::textCentered(930, 249, "no data", C_DIM);
    for (int c = 0; c < W_COUNT; c++) {
        float ly = 150 + c * 32;
        Gfx::roundRect(1040, ly - 10, 14, 14, 4, cols[c]);
        Gfx::text(1062, ly + 2, wasteInfo(c).name, C_TEXT);
        Gfx::textRight(1236, ly + 2, num(vals[c]) + " kg", C_DIM);
    }

    UI::panel(830, 400, 426, 312, "Facility Status");
    // animated sorting line: little items travelling on a conveyor, speed depends on activity
    float cy = 470;
    Gfx::roundRect(850, cy, 386, 22, 6, Color(0.12f, 0.15f, 0.2f));
    for (int k = 0; k < 9; k++) {
        float px = 850 + std::fmod(k * 44.0f + t * (rc.totalRecyclable() - rc.totalProcessed() > 0.5f ? 40.0f : 8.0f), 380.0f);
        Gfx::roundRect(px, cy + 4, 12, 14, 3, wasteInfo(k % W_COUNT).color);
    }
    // rotating gears
    float gx = 900, gy = 560;
    for (int g = 0; g < 2; g++) {
        float cxg = gx + g * 52, dir = g ? -1.0f : 1.0f;
        Gfx::circle(cxg, gy, 24, C_PANEL2, 24);
        for (int k = 0; k < 8; k++) {
            float a = (t * 40 * dir + k * 45 + g * 22) * PI_F / 180.0f;
            Gfx::rect(cxg + 24 * std::cos(a) - 3, gy + 24 * std::sin(a) - 3, 6, 6, C_TEAL);
        }
        Gfx::circle(cxg, gy, 8, C_TEAL, 14);
    }
    char b[96];
    std::snprintf(b, sizeof(b), "Delivered: %.0f kg", rc.totalReceived());
    Gfx::text(1010, 548, b, C_TEXT);
    std::snprintf(b, sizeof(b), "Recyclable: %.0f kg", rc.totalRecyclable());
    Gfx::text(1010, 570, b, C_GREEN);
    std::snprintf(b, sizeof(b), "Processed: %.0f kg", rc.totalProcessed());
    Gfx::text(1010, 592, b, C_TEXT);
    float all = rc.totalRecyclable() > 0.01f ? rc.totalProcessed() / rc.totalRecyclable() : 0;
    Gfx::text(850, 640, "Overall recycling progress", C_DIM);
    UI::progressBar(850, 650, 386, 16, all, C_GREEN);
    Gfx::text(850, 690, "Trucks unload at the Collection Center; the facility", C_DIM);
    Gfx::text(850, 706, "then sorts and recycles the recyclable share over time.", C_DIM);
}
