#include "CityMap.h"
#include "Objects.h"
#include "Gfx.h"

using namespace City;

// ---------------------------------------------------------------- camera
Mat3 Camera::matrix() const {
    return makeTranslation(MAP_X + MAP_W / 2 + panX, MAP_Y + MAP_H / 2 + panY) *
           makeScaling(zoom, zoom) *
           makeTranslation(-WORLD_W / 2, -WORLD_H / 2);
}
Vec2 Camera::screenToWorld(const Vec2& s) const { return matrix().inverse().apply(s); }
Vec2 Camera::worldToScreen(const Vec2& w) const { return matrix().apply(w); }
void Camera::clampPan() {
    float mx = WORLD_W * zoom / 2 + 100, my = WORLD_H * zoom / 2 + 100;
    panX = clampf(panX, -mx, mx);
    panY = clampf(panY, -my, my);
}
void Camera::zoomBy(float factor, const Vec2& focus) {
    Vec2 before = screenToWorld(focus);
    zoom = clampf(zoom * factor, 0.5f, 3.5f);
    Vec2 after = screenToWorld(focus);
    // keep the world point under the cursor fixed
    panX += (after.x - before.x) * zoom;
    panY += (after.y - before.y) * zoom;
    clampPan();
}

// ---------------------------------------------------------------- helpers
static Color dn(const Color& day, const Color& night, float dayFactor) { return mixColor(night, day, dayFactor); }

static void blockRect(int bx, int by, float& x, float& y, float& w, float& h) {
    x = X0 + bx * DX + ROAD_W / 2;
    y = Y0 + by * DY + ROAD_W / 2;
    w = DX - ROAD_W;
    h = DY - ROAD_W;
}

// ---------------------------------------------------------------- ground & blocks
void CityMapView::drawGround(const Simulation& sim) {
    float d = sim.dayFactor;
    Gfx::rectGradient(0, 0, WORLD_W, WORLD_H, dn(Color(0.40f, 0.68f, 0.38f), Color(0.07f, 0.16f, 0.13f), d),
                                              dn(Color(0.33f, 0.60f, 0.34f), Color(0.05f, 0.12f, 0.11f), d));
    // soft grass patches
    for (int i = 0; i < 18; i++) {
        float gx = std::fmod(i * 173.0f, WORLD_W), gy = std::fmod(i * 97.0f + 31, WORLD_H);
        Gfx::ellipse(gx, gy, 38, 16, dn(Color(0.44f, 0.72f, 0.42f, 0.35f), Color(0.09f, 0.20f, 0.15f, 0.35f), d), 14);
    }
    Gfx::rectOutline(0, 0, WORLD_W, WORLD_H, Color(0, 0, 0, 0.35f), 3);
}

void CityMapView::drawBlock(const Simulation& sim, int bx, int by) {
    float x, y, w, h;
    blockRect(bx, by, x, y, w, h);
    float d = sim.dayFactor, night = 1 - d, t = sim.time;
    BlockType type = blockType(bx, by);

    // pavement under the block
    Gfx::roundRect(x, y, w, h, 6, dn(Color(0.62f, 0.64f, 0.62f), Color(0.13f, 0.16f, 0.18f), d));
    float swayT = std::sin(t * 1.4f);

    switch (type) {
    case B_RES:
        for (int k = 0; k < 4; k++) {
            float hx = x + 16 + (k % 2) * 82, hy = y + 20 + (k / 2) * 52;
            Gfx::pushMatrix(makeTranslation(hx, hy));
            Objects::drawBuilding(BK_HOUSE, 50, 34, night, bx + by + k, t);
            Gfx::popMatrix();
        }
        for (int k = 0; k < 3; k++) {
            Gfx::pushMatrix(makeTranslation(x + 72 + k * 4, y + 38 + k * 28));
            Objects::drawTree(swayT, k); Gfx::popMatrix();
        }
        break;
    case B_COM:
        for (int k = 0; k < 2; k++) {
            Gfx::pushMatrix(makeTranslation(x + 22 + k * 86, y + 30 - k * 8));
            Objects::drawBuilding(BK_TOWER, 52, 78 + k * 16, night, k + 2, t);
            Gfx::popMatrix();
        }
        break;
    case B_HOSPITAL:
        Gfx::pushMatrix(makeTranslation(x + 24, y + 34));
        Objects::drawBuilding(BK_HOSPITAL, 112, 68, night, 1, t);
        Gfx::popMatrix();
        Gfx::circle(x + 150, y + 30, 11, Color(0.9f, 0.3f, 0.3f, 0.7f), 16);
        Gfx::textCentered(x + 150, y + 34, "H", Color(1, 1, 1), FONT_S);
        break;
    case B_SCHOOL:
        Gfx::pushMatrix(makeTranslation(x + 20, y + 36));
        Objects::drawBuilding(BK_SCHOOL, 104, 56, night, 1, t);
        Gfx::popMatrix();
        Gfx::ellipse(x + 142, y + 92, 24, 14, dn(Color(0.78f, 0.50f, 0.36f), Color(0.25f, 0.18f, 0.15f), d), 20);
        Gfx::circleOutline(x + 142, y + 92, 8, Color(1, 1, 1, 0.6f), 1.5f, 16);
        break;
    case B_PARK:
        Gfx::roundRect(x + 4, y + 4, w - 8, h - 8, 8, dn(Color(0.36f, 0.70f, 0.36f), Color(0.07f, 0.20f, 0.14f), d));
        Gfx::polyOutline(std::vector<Vec2>{ Vec2(x + 10, y + h - 14), Vec2(x + 70, y + 60), Vec2(x + 120, y + 70), Vec2(x + w - 10, y + 16) },
                         dn(Color(0.85f, 0.78f, 0.60f), Color(0.30f, 0.28f, 0.22f), d), 5, false);
        Gfx::ellipse(x + 125, y + 92, 24, 11, dn(Color(0.35f, 0.65f, 0.90f), Color(0.07f, 0.16f, 0.30f), d), 18);
        for (int k = 0; k < 9; k++) {
            Gfx::pushMatrix(makeTranslation(x + 18 + (k % 5) * 34, y + 38 + (k / 5) * 54 + (k % 2) * 8));
            Objects::drawTree(std::sin(t * 1.4f + k), k); Gfx::popMatrix();
        }
        break;
    case B_SHOP:
        Gfx::pushMatrix(makeTranslation(x + 14, y + 22));
        Objects::drawBuilding(BK_MALL, 146, 52, night, 0, t);
        Gfx::popMatrix();
        for (int k = 0; k < 6; k++)                                   // parking lot
            Gfx::rect(x + 16 + k * 25, y + 90, 17, 10, brighten(Color(0.3f + 0.1f * (k % 3), 0.4f, 0.55f + 0.07f * (k % 4)), 0.6f + 0.4f * d));
        break;
    case B_LAKE: {
        Gfx::roundRect(x + 2, y + 2, w - 4, h - 4, 14, dn(Color(0.26f, 0.58f, 0.86f), Color(0.05f, 0.14f, 0.30f), d));
        for (int k = 0; k < 6; k++) {                                // animated water waves
            float wy = y + 14 + k * 18;
            std::vector<Vec2> pts;
            for (int i = 0; i <= 14; i++) {
                float wx = x + 12 + i * ((w - 24) / 14.0f);
                pts.push_back(Vec2(wx, wy + 2.5f * std::sin(t * 2.0f + i * 0.9f + k)));
            }
            Gfx::polyOutline(pts, Color(1, 1, 1, 0.22f * d + 0.08f), 1.5f, false);
        }
        Gfx::ellipse(x + 60, y + 40, 18, 8, Color(1, 1, 1, 0.10f), 14);
        for (int k = 0; k < 3; k++) { Gfx::pushMatrix(makeTranslation(x + 20 + k * 70, y + h - 4)); Objects::drawTree(swayT, k); Gfx::popMatrix(); }
        break; }
    case B_RECYCLE:
        Gfx::pushMatrix(makeTranslation(x + 24, y + 28));
        Objects::drawBuilding(BK_RECYCLE, 120, 76, night, 0, t);
        Gfx::popMatrix();
        for (int k = 0; k < 4; k++) Gfx::rect(x + 150 + (k % 2) * 10, y + 40 + (k / 2) * 12, 8, 8, wasteInfo(k).color);
        break;
    default:
        Gfx::pushMatrix(makeTranslation(x + 20, y + 22));
        Objects::drawBuilding(BK_DEPOT, 134, 66, night, 0, t);
        Gfx::popMatrix();
        Gfx::rect(x + 6, y + h - 36, w - 12, 30, dn(Color(0.45f, 0.48f, 0.50f), Color(0.10f, 0.12f, 0.14f), d));   // parking apron
        break;
    }
    if (cam.zoom > 0.8f)
        Gfx::textCentered(x + w / 2, y + h - 4, blockName(type), withAlpha(dn(Color(0.1f, 0.2f, 0.2f), Color(0.8f, 0.9f, 0.9f), d), 0.75f), FONT_S);
}

// ---------------------------------------------------------------- roads
void CityMapView::drawRoads(const Simulation& sim) {
    const std::vector<RoadEdge>& E = sim.planner.edges();
    const std::vector<Vec2>& N = sim.planner.nodes();
    float d = sim.dayFactor;
    Color road = dn(Color(0.28f, 0.31f, 0.36f), Color(0.09f, 0.11f, 0.15f), d);
    Color edge = dn(Color(0.72f, 0.74f, 0.76f), Color(0.20f, 0.23f, 0.27f), d);

    for (size_t i = 0; i < E.size(); i++) {                               // curbs
        Vec2 a = N[E[i].a], b = N[E[i].b];
        Gfx::line(a.x, a.y, b.x, b.y, edge, ROAD_W * cam.zoom + 3);
    }
    for (size_t i = 0; i < E.size(); i++) {                               // asphalt
        Vec2 a = N[E[i].a], b = N[E[i].b];
        Gfx::line(a.x, a.y, b.x, b.y, road, ROAD_W * cam.zoom);
    }
    for (size_t i = 0; i < N.size(); i++) Gfx::rect(N[i].x - ROAD_W / 2, N[i].y - ROAD_W / 2, ROAD_W, ROAD_W, road);

    for (size_t i = 0; i < E.size(); i++) {
        Vec2 a = N[E[i].a], b = N[E[i].b];
        Vec2 dir = (b - a) * (1.0f / E[i].length);
        Vec2 nrm(-dir.y, dir.x);
        Vec2 s = a + dir * (ROAD_W / 2 + 18), e2 = b - dir * (ROAD_W / 2 + 18);
        Gfx::dashedLine(s.x, s.y, e2.x, e2.y, Color(0.95f, 0.85f, 0.3f, 0.75f), 1.5f, 9, 8);
        if (cam.zoom > 0.6f) {                                            // pedestrian crossings at both ends
            for (int end = 0; end < 2; end++) {
                Vec2 p = end == 0 ? a + dir * (ROAD_W / 2 + 7) : b - dir * (ROAD_W / 2 + 7);
                for (int k = -2; k <= 2; k++) {
                    Vec2 q = p + nrm * (float)(k * 5);
                    Gfx::line(q.x - dir.x * 4, q.y - dir.y * 4, q.x + dir.x * 4, q.y + dir.y * 4,
                              Color(1, 1, 1, 0.55f * d + 0.25f), 2.5f);
                }
            }
        }
        if (sim.traffic && City::isCongested((int)i))                     // congestion overlay
            Gfx::line(s.x, s.y, e2.x, e2.y, Color(1.0f, 0.25f, 0.15f, 0.30f + 0.12f * std::sin(sim.time * 4)), ROAD_W * cam.zoom * 0.8f);
    }
}

// ---------------------------------------------------------------- traffic lights & street lights
void CityMapView::drawLights(const Simulation& sim) {
    const std::vector<Vec2>& N = sim.planner.nodes();
    const std::vector<RoadEdge>& E = sim.planner.edges();
    float night = 1 - sim.dayFactor;
    // street lights beside the roads
    for (size_t i = 0; i < E.size(); i++) {
        Vec2 a = N[E[i].a], b = N[E[i].b];
        Vec2 dir = (b - a) * (1.0f / E[i].length), nrm(-dir.y, dir.x);
        for (int k = 1; k <= 2; k++) {
            Vec2 p = a + dir * (E[i].length * k / 3.0f) + nrm * ((k == 1 ? 1.0f : -1.0f) * (ROAD_W / 2 + 4));
            Gfx::circle(p.x, p.y, 2.2f, Color(0.25f, 0.27f, 0.30f), 8);
            Gfx::circle(p.x, p.y, 1.2f, mixColor(Color(0.8f, 0.8f, 0.75f), Color(1, 0.95f, 0.6f), night), 8);
            Objects::drawLightGlow(p.x, p.y, night);
        }
    }
    // animated traffic lights at every intersection
    for (size_t i = 0; i < N.size(); i++) {
        if ((int)i == sim.depotNode) continue;
        float ph = std::fmod(sim.time + (float)i * 1.3f, 8.0f);
        Color lamp = ph < 3.4f ? C_GREEN : (ph < 4.0f ? C_YELLOW : (ph < 7.4f ? C_RED : C_YELLOW));
        Vec2 p(N[i].x - 15, N[i].y + 13);
        Gfx::roundRect(p.x - 2.6f, p.y - 4.5f, 5.2f, 9, 2, Color(0.08f, 0.09f, 0.11f));
        Gfx::circle(p.x, p.y, 1.8f, lamp, 8);
        Gfx::circle(p.x, p.y, 4.5f, withAlpha(lamp, 0.18f), 10);
    }
}

// ---------------------------------------------------------------- routes
void CityMapView::drawRoutes(const Simulation& sim) {
    const std::vector<Vec2>& N = sim.planner.nodes();
    float pulseV = 0.5f + 0.5f * std::sin(sim.time * 5);
    // route currently driven by every working truck
    for (size_t t = 0; t < sim.trucks.size(); t++) {
        const GarbageTruck& tr = sim.trucks[t];
        if (tr.route.size() < 2) continue;
        bool sel = (int)t == sim.selTruck;
        for (size_t i = 1; i < tr.route.size(); i++) {
            Vec2 a = N[tr.route[i - 1]], b = N[tr.route[i]];
            Gfx::line(a.x, a.y, b.x, b.y, Color(tr.color.r, tr.color.g, tr.color.b, sel ? 0.55f : 0.30f), sel ? 9.0f : 6.0f);
        }
    }
    // preview of the Dijkstra result
    if (sim.previewRoute.size() >= 2) {
        std::vector<Vec2> pts;
        for (size_t i = 0; i < sim.previewRoute.size(); i++) pts.push_back(N[sim.previewRoute[i]]);
        Gfx::polyOutline(pts, Color(0.2f, 0.95f, 0.95f, 0.35f + 0.25f * pulseV), 9, false);
        for (size_t i = 1; i < pts.size(); i++)
            Gfx::dashedLine(pts[i - 1].x, pts[i - 1].y, pts[i].x, pts[i].y, Color(1, 1, 1, 0.9f), 2.5f, 8, 7);
        for (size_t i = 0; i < pts.size(); i++) Gfx::circle(pts[i].x, pts[i].y, 4, Color(0.2f, 0.95f, 0.95f), 10);
    }
}

// ---------------------------------------------------------------- clouds
void CityMapView::drawClouds(const Simulation& sim) {
    float d = sim.dayFactor;
    if (d < 0.05f) return;
    for (int i = 0; i < 4; i++) {
        float cx = std::fmod(sim.time * (7.0f + i * 3.0f) + i * 310.0f, WORLD_W + 300) - 150;
        float cy = 90 + i * 130;
        // shadow on the ground then the cloud itself
        Gfx::ellipse(cx + 14, cy + 22, 60, 16, Color(0, 0, 0, 0.10f * d), 20);
        Gfx::ellipse(cx, cy, 52, 15, Color(1, 1, 1, 0.55f * d), 20);
        Gfx::ellipse(cx - 24, cy + 4, 30, 11, Color(1, 1, 1, 0.55f * d), 16);
        Gfx::ellipse(cx + 26, cy + 3, 32, 12, Color(1, 1, 1, 0.55f * d), 16);
    }
}

// ---------------------------------------------------------------- main draw
void CityMapView::draw(const Simulation& sim) {
    Gfx::resetMatrix();
    Gfx::scissor(MAP_X, MAP_Y, MAP_W, MAP_H);
    Gfx::rect(MAP_X, MAP_Y, MAP_W, MAP_H, dn(Color(0.30f, 0.55f, 0.32f), Color(0.03f, 0.07f, 0.07f), sim.dayFactor));

    Gfx::pushMatrix(cam.matrix());
    drawGround(sim);
    for (int by = 0; by < 3; by++) for (int bx = 0; bx < 4; bx++) drawBlock(sim, bx, by);
    drawRoads(sim);
    drawRoutes(sim);
    drawLights(sim);

    float night = 1 - sim.dayFactor;
    // ambient traffic
    if (sim.traffic) {
        for (size_t i = 0; i < sim.cars.size(); i++) {
            const AmbientCar& c = sim.cars[i];
            float a = c.mover.angle * PI_F / 180.0f;
            Vec2 p = c.mover.pos + Vec2(-std::sin(a), std::cos(a)) * 5.0f;     // drive in the right lane
            Gfx::pushMatrix(makeTranslation(p.x, p.y) * makeRotation(c.mover.angle));
            Objects::drawCar(c.color, night);
            Gfx::popMatrix();
        }
    }
    // dustbins
    for (size_t i = 0; i < sim.waste.bins.size(); i++) {
        const Dustbin& b = sim.waste.bins[i];
        float lift = (b.status == CS_COLLECTING) ? 3.5f + 2.0f * std::sin(sim.time * 14) : 0.0f;
        Gfx::pushMatrix(makeTranslation(b.pos.x, b.pos.y) * makeScaling(1.35f, 1.35f));
        Objects::drawBin(b.levelColor(), wasteInfo(b.category).color, b.level / b.capacity, (int)i == sim.selBin,
                         b.flash > 0, lift, sim.time);
        Gfx::popMatrix();
        if (cam.zoom > 0.9f) Gfx::textCentered(b.pos.x, b.pos.y + 20, b.label(), Color(1, 1, 1, 0.85f), FONT_S);
    }
    // trucks
    for (size_t i = 0; i < sim.trucks.size(); i++) {
        const GarbageTruck& t = sim.trucks[i];
        float a = t.mover.angle * PI_F / 180.0f;
        Vec2 p = t.mover.pos;
        if (t.state == T_TO_BIN || t.state == T_RETURNING) p = p + Vec2(-std::sin(a), std::cos(a)) * 4.0f;
        Gfx::pushMatrix(makeTranslation(p.x, p.y) * makeRotation(t.mover.angle));
        Objects::drawTruck(t.color, t.wheelAngle, t.cargoTotal() / 300.0f, night);
        Gfx::popMatrix();
        if ((int)i == sim.selTruck) {
            Gfx::circleOutline(p.x, p.y, 28 + 2 * std::sin(sim.time * 6), Color(1, 1, 1, 0.9f), 2, 28);
        }
        Gfx::textCentered(p.x, p.y - 17, t.label(), Color(1, 1, 1), FONT_S);
        if (t.state == T_COLLECTING) {                                  // collection progress ring
            Gfx::arc(p.x, p.y, 22, 26, -90, -90 + 360 * t.progress, C_YELLOW);
        }
    }
    drawClouds(sim);
    if (night > 0.01f) Gfx::rect(0, 0, WORLD_W, WORLD_H, Color(0.02f, 0.03f, 0.12f, 0.20f * night));   // night tint
    Gfx::popMatrix();

    drawSelectedPopup(sim);
    drawOverlayUI(sim);
    Gfx::scissorOff();
    Gfx::rectOutline(MAP_X + 0.5f, MAP_Y + 0.5f, MAP_W - 1, MAP_H - 1, C_BORDER, 2);
}

// ---------------------------------------------------------------- screen-space overlays
void CityMapView::drawSelectedPopup(const Simulation& sim) {
    if (sim.selBin < 0) return;
    const Dustbin& b = sim.waste.bins[sim.selBin];
    Vec2 s = cam.worldToScreen(b.pos);
    float pw = 170, ph = 92;
    float px = clampf(s.x + 18, MAP_X + 6, MAP_X + MAP_W - pw - 6);
    float py = clampf(s.y - ph - 14, MAP_Y + 6, MAP_Y + MAP_H - ph - 6);
    Gfx::roundRect(px + 3, py + 3, pw, ph, 8, Color(0, 0, 0, 0.35f));
    Gfx::roundRectGradient(px, py, pw, ph, 8, C_PANEL2, C_PANEL);
    Gfx::roundRectOutline(px, py, pw, ph, 8, b.levelColor(), 1.5f);
    Gfx::text(px + 10, py + 18, b.label() + "  -  " + wasteInfo(b.category).name, C_TEXT);
    float pct = b.percent();
    Gfx::text(px + 10, py + 36, "Level: " + std::to_string((int)pct) + "%  (" + b.levelName() + ")", b.levelColor());
    Gfx::roundRect(px + 10, py + 44, pw - 20, 8, 4, Color(0, 0, 0, 0.4f));
    Gfx::roundRect(px + 10, py + 44, std::max(6.0f, (pw - 20) * pct / 100.0f), 8, 4, b.levelColor());
    char buf[64];
    std::snprintf(buf, sizeof(buf), "%.0f / %.0f kg", b.level, b.capacity);
    Gfx::text(px + 10, py + 68, buf, C_DIM);
    Gfx::text(px + 10, py + 84, b.statusName(), C_TEXT);
    if (b.levelClass() == 3) Gfx::textRight(px + pw - 10, py + 18, "FULL!", C_RED);
}

void CityMapView::drawOverlayUI(const Simulation& sim) {
    // zoom indicator + hint
    char z[48];
    std::snprintf(z, sizeof(z), "Zoom %d%%", (int)(cam.zoom * 100));
    Gfx::roundRect(MAP_X + 10, MAP_Y + 10, 92, 24, 6, Color(0.03f, 0.06f, 0.10f, 0.8f));
    Gfx::text(MAP_X + 20, MAP_Y + 27, z, C_TEXT);
    Gfx::roundRect(MAP_X + 110, MAP_Y + 10, 292, 24, 6, Color(0.03f, 0.06f, 0.10f, 0.8f));
    Gfx::text(MAP_X + 120, MAP_Y + 27, "Wheel: zoom  |  Drag: pan  |  Click: select", C_DIM);

    // status badge
    const char* st = !sim.started ? "NOT STARTED" : (sim.paused ? "PAUSED" : "RUNNING");
    Color sc = !sim.started ? C_YELLOW : (sim.paused ? C_ORANGE : C_GREEN);
    Gfx::roundRect(MAP_X + MAP_W - 150, MAP_Y + 10, 140, 24, 6, Color(0.03f, 0.06f, 0.10f, 0.8f));
    Gfx::circle(MAP_X + MAP_W - 136, MAP_Y + 22, 4, sc, 10);
    Gfx::text(MAP_X + MAP_W - 126, MAP_Y + 27, st, sc);

    // legend
    float lx = MAP_X + 10, ly = MAP_Y + MAP_H - 34;
    Gfx::roundRect(lx, ly, 330, 24, 6, Color(0.03f, 0.06f, 0.10f, 0.8f));
    static const char* names[4] = { "Low", "Medium", "High", "Full" };
    Color cs[4] = { C_GREEN, C_YELLOW, C_ORANGE, C_RED };
    for (int i = 0; i < 4; i++) {
        Gfx::circle(lx + 14 + i * 76, ly + 12, 5, cs[i], 10);
        Gfx::text(lx + 24 + i * 76, ly + 17, names[i], C_TEXT);
    }

    // toast message
    if (sim.toastTimer > 0) {
        float a = clampf(sim.toastTimer, 0, 1);
        float tw = (float)Gfx::textWidth(sim.toast, FONT_S) + 30;
        float tx = MAP_X + MAP_W / 2 - tw / 2, ty = MAP_Y + MAP_H - 70;
        Gfx::roundRect(tx, ty, tw, 30, 8, Color(0.05f, 0.30f, 0.24f, 0.92f * a));
        Gfx::roundRectOutline(tx, ty, tw, 30, 8, Color(0.17f, 0.80f, 0.45f, a), 1.5f);
        Gfx::text(tx + 15, ty + 20, sim.toast, Color(1, 1, 1, a));
    }
}

int CityMapView::pickBin(const Simulation& sim, const Vec2& screen) const {
    Vec2 w = cam.screenToWorld(screen);
    int best = -1; float bd = 18.0f;
    for (size_t i = 0; i < sim.waste.bins.size(); i++) {
        float d = distf(w, sim.waste.bins[i].pos);
        if (d < bd) { bd = d; best = (int)i; }
    }
    return best;
}

int CityMapView::pickTruck(const Simulation& sim, const Vec2& screen) const {
    Vec2 w = cam.screenToWorld(screen);
    int best = -1; float bd = 24.0f;
    for (size_t i = 0; i < sim.trucks.size(); i++) {
        float d = distf(w, sim.trucks[i].mover.pos);
        if (d < bd) { bd = d; best = (int)i; }
    }
    return best;
}
