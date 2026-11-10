#include "Application.h"
#include "GLInclude.h"
#include "Gfx.h"
#include "Objects.h"
#include "Statistics.h"

enum {
    BTN_S_START = 1, BTN_S_ABOUT, BTN_S_EXIT,
    BTN_NAV_DASH = 10, BTN_NAV_MAP, BTN_NAV_RECYCLE, BTN_NAV_STATS, BTN_NAV_LAB, BTN_NAV_HOME,
    BTN_C_START = 30, BTN_C_PAUSE, BTN_C_RESUME, BTN_C_RESET, BTN_C_ADD, BTN_C_EMPTY, BTN_C_DISPATCH,
    BTN_C_ROUTE, BTN_C_TRAFFIC, BTN_C_DAYNIGHT, BTN_C_TRANSFORM, BTN_C_AUTO, BTN_C_CATEGORY,
    BTN_REPORT = 50
};

Application::Application()
    : wantsExit(false), screen(SCR_SPLASH), showAbout(false), showReport(false),
      mx(0), my(0), winW(WIN_W), winH(WIN_H), dragging(false), dragMoved(false),
      pressX(0), pressY(0), lastX(0), lastY(0) {}

void Application::init() {
    sim.init();
    buildButtons();
}

void Application::setWindowSize(int w, int h) {
    winW = w > 1 ? w : 1;
    winH = h > 1 ? h : 1;
    Gfx::setWindowSize(winW, winH);
}

Vec2 Application::toLogical(int x, int y) const {
    return Vec2(x * (float)WIN_W / winW, y * (float)WIN_H / winH);
}

void Application::buildButtons() {
    splashButtons.clear(); navButtons.clear(); controlButtons.clear(); reportButton.clear();
    splashButtons.push_back(Button(BTN_S_START, 490, 360, 300, 52, "Start Simulation", C_GREEN));
    splashButtons.push_back(Button(BTN_S_ABOUT, 490, 424, 300, 52, "About Project", C_BLUE));
    splashButtons.push_back(Button(BTN_S_EXIT,  490, 488, 300, 52, "Exit", C_RED));

    const char* names[4] = { "Dashboard", "City Map", "Recycling", "Statistics" };
    for (int i = 0; i < 4; i++) navButtons.push_back(Button(BTN_NAV_DASH + i, 300 + i * 136, 11, 128, 34, names[i], C_TEAL));
    navButtons.push_back(Button(BTN_NAV_HOME, 1186, 11, 80, 34, "Home", C_BLUE));

    struct Def { int id; const char* label; Color c; };
    Def defs[11] = {
        { BTN_C_START, "Start Simulation", C_GREEN }, { BTN_C_PAUSE, "Pause Simulation", C_ORANGE },
        { BTN_C_RESUME, "Resume Simulation", C_GREEN }, { BTN_C_RESET, "Reset Simulation", C_RED },
        { BTN_C_ADD, "Add Waste", C_YELLOW }, { BTN_C_EMPTY, "Empty Selected Bin", C_TEAL },
        { BTN_C_DISPATCH, "Dispatch Garbage Truck", C_GREEN }, { BTN_C_ROUTE, "Find Shortest Route", C_BLUE },
        { BTN_C_TRAFFIC, "Toggle Traffic", C_ORANGE }, { BTN_C_DAYNIGHT, "Toggle Day/Night", C_BLUE },
        { BTN_C_AUTO, "Auto Dispatch", C_GREEN }
    };
    for (int i = 0; i < 11; i++)
        controlButtons.push_back(Button(defs[i].id, 972 + (i % 2) * 150, 98 + (i / 2) * 40, 144, 34, defs[i].label, defs[i].c));
    categoryButton = Button(BTN_C_CATEGORY, 972, 344 + 182 - 38, 296, 30, "Change Waste Category", C_TEAL);
    reportButton.push_back(Button(BTN_REPORT, 1060, 16, 196, 30, "Generate Report", C_GREEN));
}

// ------------------------------------------------------------------ update / render
void Application::update(float dt) {
    sim.update(dt);
    dashboard.update(dt, sim);
}

void Application::render() {
    glClearColor(C_BG.r, C_BG.g, C_BG.b, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    Gfx::setupProjection();
    Gfx::resetMatrix();

    if (screen == SCR_SPLASH) {
        drawSplash();
    } else {
        UI::background(sim.time);
        switch (screen) {
            case SCR_DASH:    dashboard.draw(sim); break;
            case SCR_MAP:     drawMapScreen(); break;
            case SCR_RECYCLE: dashboard.drawRecycling(sim); break;
            case SCR_STATS: {
                Statistics::draw(sim);
                UI::button(reportButton[0], reportButton[0].contains(mx, my));
                break; }
            default:          lab.draw(sim.time, mx, my); break;
        }
        drawNav();
    }
    if (showAbout) drawAbout();
    if (showReport) Statistics::drawReportOverlay(sim);
}

// ------------------------------------------------------------------ nav bar
void Application::drawNav() {
    Gfx::rectGradient(0, 0, WIN_W, 56, Color(0.06f, 0.11f, 0.18f), Color(0.04f, 0.08f, 0.13f));
    Gfx::line(0, 56, WIN_W, 56, C_BORDER, 2);
    UI::logo(32, 28, 17, sim.time);
    Gfx::text(58, 34, "CleanCity Simulator", C_TEXT, FONT_M);
    for (size_t i = 0; i < navButtons.size(); i++) {
        Button b = navButtons[i];
        b.toggled = (b.id - BTN_NAV_DASH == (int)screen - 1) && b.id != BTN_NAV_HOME;
        UI::button(b, b.contains(mx, my));
    }
    const char* st = !sim.started ? "IDLE" : (sim.paused ? "PAUSED" : "RUNNING");
    Color sc = !sim.started ? C_YELLOW : (sim.paused ? C_ORANGE : C_GREEN);
    Gfx::circle(1010, 28, 5, sc, 12);
    char buf[48];
    std::snprintf(buf, sizeof(buf), "%s  x%.0f", st, sim.speed);
    Gfx::text(1022, 33, buf, sc);
    Gfx::text(1118, 33, sim.dayFactor > 0.5f ? "DAY" : "NIGHT", C_DIM);
}

// ------------------------------------------------------------------ map screen
void Application::drawMapScreen() {
    mapView.draw(sim);
    drawControlPanel();
}

void Application::drawControlPanel() {
    UI::panel(962, 64, 310, 276, "Control Panel");
    for (size_t i = 0; i < controlButtons.size(); i++) {
        Button b = controlButtons[i];
        switch (b.id) {
            case BTN_C_START:   b.enabled = !sim.started; break;
            case BTN_C_PAUSE:   b.enabled = sim.running(); break;
            case BTN_C_RESUME:  b.enabled = sim.started && sim.paused; break;
            case BTN_C_TRAFFIC: b.toggled = sim.traffic; break;
            case BTN_C_DAYNIGHT: b.toggled = sim.dayTarget < 0.5f; break;
            case BTN_C_AUTO:    b.toggled = sim.autoDispatch; break;
            default: break;
        }
        UI::button(b, b.contains(mx, my));
    }

    // ---- information card (selected dustbin / truck)
    float cy = 344, ch = 182;
    UI::panel(962, cy, 310, ch);
    char buf[128];
    if (sim.selBin >= 0) {
        const Dustbin& b = sim.waste.bins[sim.selBin];
        Gfx::text(976, cy + 22, b.label() + " - " + wasteInfo(b.category).name + " bin", C_TEXT, FONT_M);
        Gfx::textRight(1256, cy + 22, b.levelName(), b.levelColor(), FONT_S);
        UI::progressBar(976, cy + 32, 282, 10, b.percent() / 100.0f, b.levelColor());
        std::snprintf(buf, sizeof(buf), "Level: %.0f%% (%.0f / %.0f kg)", b.percent(), b.level, b.capacity);
        Gfx::text(976, cy + 62, buf, C_TEXT);
        Gfx::text(976, cy + 80, std::string("Zone: ") + WasteManager::zoneName(b.zone), C_DIM);
        Gfx::text(976, cy + 98, std::string("Status: ") + b.statusName(), C_DIM);
        std::snprintf(buf, sizeof(buf), "Location: (%.0f, %.0f)", b.pos.x, b.pos.y);
        Gfx::text(976, cy + 116, buf, C_DIM);
        if (b.levelClass() == 3) Gfx::text(976, cy + 134, "WARNING: bin is full - dispatch a truck!", C_RED);
        else if (sim.previewRoute.size() > 1) {
            std::snprintf(buf, sizeof(buf), "Route: %.0f units (%d nodes explored)", sim.previewDist, sim.previewVisited);
            Gfx::text(976, cy + 134, buf, C_BLUE);
        }
        UI::button(categoryButton, categoryButton.contains(mx, my));
    } else if (sim.selTruck >= 0) {
        const GarbageTruck& t = sim.trucks[sim.selTruck];
        Gfx::text(976, cy + 22, "Truck " + t.label(), C_TEXT, FONT_M);
        Gfx::text(976, cy + 50, "Status: " + t.statusText(), C_TEXT);
        std::snprintf(buf, sizeof(buf), "Collections done: %d", t.collections);
        Gfx::text(976, cy + 70, buf, C_DIM);
        std::snprintf(buf, sizeof(buf), "Cargo on board: %.0f kg", t.cargoTotal());
        Gfx::text(976, cy + 88, buf, C_DIM);
        if (t.route.size() > 1) {
            std::snprintf(buf, sizeof(buf), "Route distance: %.0f units", t.routeDist);
            Gfx::text(976, cy + 106, buf, C_BLUE);
        }
        Gfx::text(976, cy + 140, "Click a dustbin, then Dispatch.", C_DIM);
    } else {
        Gfx::text(976, cy + 22, "Selection", C_TEXT, FONT_M);
        Gfx::text(976, cy + 52, "Click a dustbin to see its details.", C_DIM);
        Gfx::text(976, cy + 72, "Click a truck to select it.", C_DIM);
        Gfx::text(976, cy + 100, "Keys: Space pause, N night,", C_DIM);
        Gfx::text(976, cy + 116, "T traffic, A add, E empty,", C_DIM);
        Gfx::text(976, cy + 132, "D dispatch, R route, S speed", C_DIM);
        Gfx::text(976, cy + 148, "U auto, +/- zoom, 0 reset view", C_DIM);
    }

    // ---- activity log
    float ay = cy + ch + 8, ah = WIN_H - ay - 8;
    UI::panel(962, ay, 310, ah, "Activity");
    int rows = (int)((ah - 44) / 17), shown = 0;
    for (int i = (int)sim.waste.log.size() - 1; i >= 0 && shown < rows; i--, shown++) {
        std::string s = sim.waste.log[i];
        while (Gfx::textWidth(s) > 284 && s.size() > 4) s.erase(s.size() - 1);
        Gfx::text(976, ay + 48 + shown * 17, s, withAlpha(C_TEXT, 1.0f - shown * 0.1f));
    }
}

// ------------------------------------------------------------------ splash
void Application::drawSplash() {
    float t = sim.time;
    Gfx::rectGradient(0, 0, WIN_W, WIN_H, Color(0.02f, 0.05f, 0.14f), Color(0.04f, 0.26f, 0.30f));
    for (int i = 0; i < 70; i++) {                                        // twinkling stars
        float sx = std::fmod(i * 197.0f, WIN_W), sy = std::fmod(i * 61.0f, 300.0f);
        Gfx::point(sx, sy, 1.5f + (i % 3) * 0.5f, Color(1, 1, 1, 0.3f + 0.5f * std::fabs(std::sin(t * 1.5f + i))));
    }
    Gfx::circle(1110, 110, 72, Color(0.8f, 0.9f, 1.0f, 0.04f), 40); Gfx::circle(1110, 110, 56, Color(0.8f, 0.9f, 1.0f, 0.06f), 40); Gfx::circle(1110, 110, 42, Color(0.8f, 0.9f, 1.0f, 0.10f), 40);
    Gfx::circle(1110, 110, 30, Color(0.96f, 0.96f, 0.88f), 40); Gfx::circle(1099, 100, 7, Color(0.82f, 0.84f, 0.80f), 20); Gfx::circle(1121, 119, 9, Color(0.84f, 0.86f, 0.82f), 20); Gfx::circle(1117, 97, 4, Color(0.84f, 0.86f, 0.82f), 16); Gfx::circle(1100, 122, 5, Color(0.86f, 0.88f, 0.84f), 16);
    
    for (int i = 0; i < 4; i++) {                                          // moving clouds
        float cx = std::fmod(t * (10 + i * 5) + i * 400.0f, WIN_W + 300) - 150, cy = 90 + i * 55;
        Gfx::ellipse(cx, cy, 70, 16, Color(1, 1, 1, 0.10f), 20);
        Gfx::ellipse(cx - 30, cy + 5, 40, 12, Color(1, 1, 1, 0.10f), 16);
    }
    // skyline, two layers
    for (int layer = 0; layer < 2; layer++) {
        for (int i = 0; i < 26; i++) {
            float bw = 54, bx = i * bw - layer * 27;
            float bh = 80 + std::fmod((float)(i * 37 + layer * 53), 150.0f) + (layer ? 0 : 40);
            float by = 625 - bh;
            Color body = layer ? Color(0.05f, 0.14f, 0.22f) : Color(0.03f, 0.08f, 0.14f);
            Gfx::rectGradient(bx, by, bw - 4, bh, Color(body.r + 0.05f, body.g + 0.09f, body.b + 0.12f), body); Gfx::rect(bx + bw - 8, by, 4, bh, Color(0, 0, 0, 0.18f)); if (i % 3 == 0) { float ax = bx + (bw - 4) / 2; Gfx::line(ax, by, ax, by - 20, Color(0.45f, 0.5f, 0.58f), 2); Gfx::point(ax, by - 20, 5, Color(1, 0.25f, 0.2f, 0.5f + 0.5f * std::sin(t * 3 + i))); } else if (i % 4 == 1) Gfx::rect(bx + 10, by - 10, bw - 24, 10, body);
            for (int wy = 0; wy < (int)(bh / 16); wy++)
                for (int wx = 0; wx < 3; wx++) {
                    bool on = ((i * 5 + wy * 3 + wx * 7 + (int)(t * 0.4f)) % 4) != 0;
                    if (on) { bool cool = ((i + wy * 2 + wx) % 6) == 0; Color wc = cool ? Color(0.7f, 0.88f, 1, 0.8f) : Color(1, 0.88f, 0.45f, 0.9f); if (layer) wc.a *= 0.6f; Gfx::rect(bx + 6 + wx * 15, by + 8 + wy * 16, 8, 8, wc); }
                }
        }
    }
    Gfx::rectGradient(0, 625, WIN_W, 95, Color(0.07f, 0.09f, 0.13f), Color(0.03f, 0.05f, 0.08f));
    for (float x = -60 + std::fmod(t * 80, 60.0f); x < WIN_W; x += 60) Gfx::rect(x, 672, 32, 4, Color(0.95f, 0.85f, 0.3f, 0.8f));
    float tx = std::fmod(t * 95, WIN_W + 260) - 130;                      // truck driving through the city
    Gfx::pushMatrix(makeTranslation(tx, 652) * makeScaling(2.0f, 2.0f));
    Objects::drawTruck(C_GREEN, t * 400, 0.6f, 1.0f);
    Gfx::popMatrix();
    Gfx::rect(0, 0, WIN_W, WIN_H, Color(0, 0.03f, 0.08f, 0.30f));

    UI::logo(640, 112, 50, t);
    float w1 = Gfx::strokeWidth("CLEANCITY ", 0.46f), w2 = Gfx::strokeWidth("SIMULATOR", 0.46f);
    float x0 = 640 - (w1 + w2) / 2;
    Gfx::strokeText(x0, 232, "CLEANCITY ", 0.46f, C_TEXT, 3);
    Gfx::strokeText(x0 + w1, 232, "SIMULATOR", 0.46f, C_GREEN, 3);
    Gfx::textCentered(640, 272, "Smart City Waste Management & Cleanliness Monitoring Simulation", C_TEXT, FONT_M);
    Gfx::textCentered(640, 304, "100% software simulation   |   C++ & OpenGL   |   Dijkstra routing", C_DIM);

    for (size_t i = 0; i < splashButtons.size(); i++)
        UI::button(splashButtons[i], splashButtons[i].contains(mx, my), FONT_M);
    Gfx::textCentered(640, 700, "Computer Graphics Project", withAlpha(C_TEXT, 0.6f));
}

void Application::drawAbout() {
    Gfx::rect(0, 0, WIN_W, WIN_H, Color(0, 0, 0, 0.7f));
    float w = 780, h = 520, x = (WIN_W - w) / 2, y = (WIN_H - h) / 2;
    UI::panel(x, y, w, h, "About CleanCity Simulator");
    const char* L[] = {
        "CleanCity Simulator simulates a smart city's waste collection system in a",
        "completely software-based environment. There is no hardware, no sensors,",
        "no GPS - every dustbin, truck and event is generated by the simulation.",
        "",
        "Computer Graphics concepts demonstrated:",
        "  - 2D translation, rotation, scaling, reflection and shearing (3x3 matrices)",
        "  - Time based animation: trucks, wheels, traffic lights, clouds, water",
        "  - Mouse picking, zoom, pan and keyboard control",
        "  - Lines, points, triangles, quads, polygons and colour interpolation",
        "",
        "Algorithms:",
        "  - Dijkstra's shortest path on the road graph (routes are really computed)",
        "",
        "How to use: Start Simulation, click a dustbin on the City Map, then use",
        "Find Shortest Route and Dispatch Garbage Truck.",
        
        "",
        "Click anywhere or press Esc to close."
    };
    for (int i = 0; i < (int)(sizeof(L) / sizeof(L[0])); i++) Gfx::text(x + 28, y + 62 + i * 24, L[i], i == 4 || i == 10 ? C_GREEN : C_TEXT);
}

// ------------------------------------------------------------------ input
void Application::handleButton(int id) {
    switch (id) {
        case BTN_S_START: sim.cmdStart(); screen = SCR_MAP; break;
        case BTN_S_ABOUT: showAbout = true; break;
        case BTN_S_EXIT:  wantsExit = true; break;
        case BTN_NAV_DASH: screen = SCR_DASH; break;
        case BTN_NAV_MAP: screen = SCR_MAP; break;
        case BTN_NAV_RECYCLE: screen = SCR_RECYCLE; break;
        case BTN_NAV_STATS: screen = SCR_STATS; break;
        case BTN_NAV_LAB: screen = SCR_LAB; break;
        case BTN_NAV_HOME: screen = SCR_SPLASH; break;
        case BTN_C_START: sim.cmdStart(); break;
        case BTN_C_PAUSE: sim.cmdPause(); break;
        case BTN_C_RESUME: sim.cmdResume(); break;
        case BTN_C_RESET: sim.reset(); break;
        case BTN_C_ADD: sim.cmdAddWaste(); break;
        case BTN_C_EMPTY: sim.cmdEmptySelected(); break;
        case BTN_C_DISPATCH: sim.cmdDispatch(); break;
        case BTN_C_ROUTE: sim.cmdFindRoute(); break;
        case BTN_C_TRAFFIC: sim.cmdToggleTraffic(); break;
        case BTN_C_DAYNIGHT: sim.cmdToggleDayNight(); break;
        case BTN_C_TRANSFORM: screen = SCR_LAB; break;
        case BTN_C_AUTO: sim.cmdToggleAuto(); break;
        case BTN_C_CATEGORY: sim.cmdCycleCategory(); break;
        case BTN_REPORT: Statistics::saveReport(sim, "report_summary.txt"); showReport = true; break;
        default: break;
    }
}

void Application::mapClick(float x, float y) {
    Vec2 s(x, y);
    int b = mapView.pickBin(sim, s);
    if (b >= 0) { sim.selBin = b; return; }
    int t = mapView.pickTruck(sim, s);
    if (t >= 0) { sim.selTruck = t; sim.selBin = -1; return; }
    sim.selBin = -1;                                    // click on empty ground clears the selection
    sim.selTruck = -1;
}

void Application::onMouse(int button, int state, int x, int y) {
    Vec2 p = toLogical(x, y);
    mx = p.x; my = p.y;
    if (state == GLUT_DOWN && (button == 3 || button == 4)) { onWheel(button == 3 ? 1 : -1, x, y); return; }
    if (button != GLUT_LEFT_BUTTON) return;

    if (state == GLUT_DOWN) {
        if (showAbout || showReport) { showAbout = showReport = false; return; }
        const std::vector<Button>* lists[3] = { 0, 0, 0 };
        if (screen == SCR_SPLASH) lists[0] = &splashButtons;
        else { lists[0] = &navButtons; if (screen == SCR_MAP) lists[1] = &controlButtons; if (screen == SCR_STATS) lists[2] = &reportButton; }
        for (int l = 0; l < 3; l++) {
            if (!lists[l]) continue;
            for (size_t i = 0; i < lists[l]->size(); i++) {
                const Button& b = (*lists[l])[i];
                bool enabled = b.enabled;
                if (b.id == BTN_C_START) enabled = !sim.started;
                if (b.id == BTN_C_PAUSE) enabled = sim.running();
                if (b.id == BTN_C_RESUME) enabled = sim.started && sim.paused;
                if (b.contains(p.x, p.y) && enabled) { handleButton(b.id); return; }
            }
        }
        if (screen == SCR_MAP && sim.selBin >= 0 && categoryButton.contains(p.x, p.y)) { handleButton(BTN_C_CATEGORY); return; }
        if (screen == SCR_LAB) { lab.click(p.x, p.y, sim.time); return; }
        if (screen == SCR_MAP && CityMapView::inside(p.x, p.y)) {
            dragging = true; dragMoved = false;
            pressX = lastX = p.x; pressY = lastY = p.y;
        }
    } else if (state == GLUT_UP) {
        if (dragging && !dragMoved) mapClick(pressX, pressY);
        dragging = false;
    }
}

void Application::onMotion(int x, int y) {
    Vec2 p = toLogical(x, y);
    mx = p.x; my = p.y;
    if (dragging) {
        if (std::fabs(p.x - pressX) + std::fabs(p.y - pressY) > 5) dragMoved = true;
        if (dragMoved) {
            mapView.cam.panX += p.x - lastX;
            mapView.cam.panY += p.y - lastY;
            mapView.cam.clampPan();
        }
        lastX = p.x; lastY = p.y;
    }
}

void Application::onPassiveMotion(int x, int y) {
    Vec2 p = toLogical(x, y);
    mx = p.x; my = p.y;
}

void Application::onWheel(int dir, int x, int y) {
    Vec2 p = toLogical(x, y);
    if (screen == SCR_MAP && CityMapView::inside(p.x, p.y))
        mapView.cam.zoomBy(dir > 0 ? 1.12f : 1.0f / 1.12f, p);
}

void Application::onKey(unsigned char key) {
    Vec2 center(MAP_X + MAP_W / 2, MAP_Y + MAP_H / 2);
    switch (key) {
        case 27:
            if (showAbout || showReport) showAbout = showReport = false;
            else if (screen == SCR_SPLASH) wantsExit = true;
            else screen = SCR_SPLASH;
            break;
        case '1': if (screen != SCR_SPLASH) screen = SCR_DASH; break;
        case '2': if (screen != SCR_SPLASH) screen = SCR_MAP; break;
        case '3': if (screen != SCR_SPLASH) screen = SCR_RECYCLE; break;
        case '4': if (screen != SCR_SPLASH) screen = SCR_STATS; break;
        
        case ' ': if (!sim.started) sim.cmdStart(); else if (sim.paused) sim.cmdResume(); else sim.cmdPause(); break;
        case '+': case '=': mapView.cam.zoomBy(1.15f, center); break;
        case '-': case '_': mapView.cam.zoomBy(1.0f / 1.15f, center); break;
        case '0': mapView.cam.reset(); break;
        case 'n': case 'N': sim.cmdToggleDayNight(); break;
        case 't': case 'T': sim.cmdToggleTraffic(); break;
        case 'a': case 'A': sim.cmdAddWaste(); break;
        case 'e': case 'E': sim.cmdEmptySelected(); break;
        case 'd': case 'D': sim.cmdDispatch(); break;
        case 'r': case 'R': sim.cmdFindRoute(); break;
        case 's': case 'S': sim.cmdCycleSpeed(); break;
        case 'u': case 'U': sim.cmdToggleAuto(); break;
        default: break;
    }
}

void Application::onSpecial(int key) {
    float step = 40;
    switch (key) {
        case GLUT_KEY_LEFT:  mapView.cam.panX += step; break;
        case GLUT_KEY_RIGHT: mapView.cam.panX -= step; break;
        case GLUT_KEY_UP:    mapView.cam.panY += step; break;
        case GLUT_KEY_DOWN:  mapView.cam.panY -= step; break;
        default: break;
    }
    mapView.cam.clampPan();
}
