#include "Objects.h"
#include "WasteManager.h"

namespace Objects {

static Color wasteInfo2Color(int k) { return wasteInfo(k % W_COUNT).color; }

static Color windowColor(float night, int r, int c, int variant) {
    Color day(0.62f, 0.80f, 0.92f), lit(1.0f, 0.86f, 0.42f), off(0.10f, 0.14f, 0.22f);
    bool on = ((r * 7 + c * 3 + variant) % 3) != 0;
    return mixColor(day, on ? lit : off, night);
}

// 2.5D box: front face + oblique top + right side
static void box(float w, float h, float sd, const Color& front, const Color& top, const Color& side) {
    std::vector<Vec2> t, s;
    t.push_back(Vec2(0, 0)); t.push_back(Vec2(sd, -sd)); t.push_back(Vec2(w + sd, -sd)); t.push_back(Vec2(w, 0));
    s.push_back(Vec2(w, 0)); s.push_back(Vec2(w + sd, -sd)); s.push_back(Vec2(w + sd, h - sd)); s.push_back(Vec2(w, h));
    Gfx::rect(0, 0, w, h, front);
    Gfx::poly(t, top);
    Gfx::poly(s, side);
}

static void windows(float x, float y, float w, float h, int rows, int cols, float night, int variant) {
    float cw = w / cols, rh = h / rows;
    for (int r = 0; r < rows; r++)
        for (int c = 0; c < cols; c++)
            Gfx::rect(x + c * cw + cw * 0.2f, y + r * rh + rh * 0.2f, cw * 0.6f, rh * 0.6f, windowColor(night, r, c, variant));
}

void drawLightGlow(float x, float y, float night) {
    if (night < 0.05f) return;
    Gfx::circle(x, y, 26, Color(1.0f, 0.85f, 0.4f, 0.07f * night), 20);
    Gfx::circle(x, y, 15, Color(1.0f, 0.85f, 0.4f, 0.13f * night), 20);
    Gfx::circle(x, y, 7, Color(1.0f, 0.92f, 0.6f, 0.30f * night), 14);
}

void drawBuilding(int kind, float w, float h, float night, int variant, float time) {
    float dim = 1.0f - 0.55f * night;                  // buildings get darker at night
    Gfx::ellipse(w / 2 + 4, h + 1, w / 2 + 6, 4, Color(0, 0, 0, 0.22f), 20);   // shadow
    switch (kind) {
    case BK_HOUSE: {
        static const Color walls[3] = { Color(0.93f, 0.85f, 0.72f), Color(0.80f, 0.88f, 0.93f), Color(0.92f, 0.78f, 0.78f) };
        static const Color roofs[3] = { Color(0.72f, 0.28f, 0.22f), Color(0.30f, 0.38f, 0.55f), Color(0.45f, 0.30f, 0.25f) };
        Color wall = brighten(walls[variant % 3], dim), roof = brighten(roofs[variant % 3], dim);
        float rh = h * 0.40f;
        Gfx::rect(0, rh, w, h - rh, wall);
        Gfx::poly(std::vector<Vec2>{ Vec2(-3, rh), Vec2(w / 2, 0), Vec2(w + 3, rh) }, roof);
        Gfx::rect(w * 0.42f, h - h * 0.38f, w * 0.16f, h * 0.38f, brighten(Color(0.45f, 0.30f, 0.20f), dim));
        Gfx::rect(w * 0.10f, rh + 4, w * 0.22f, h * 0.22f, windowColor(night, 0, variant, variant));
        Gfx::rect(w * 0.68f, rh + 4, w * 0.22f, h * 0.22f, windowColor(night, 1, variant, variant));
        break; }
    case BK_TOWER: {
        Color f = brighten(Color(0.42f, 0.58f, 0.75f), dim), t = brighten(Color(0.60f, 0.74f, 0.88f), dim), s = brighten(Color(0.26f, 0.38f, 0.52f), dim);
        box(w, h, 8, f, t, s);
        windows(3, 4, w - 6, h - 10, (int)(h / 11), 4, night, variant);
        Gfx::rect(w * 0.35f, h - 7, w * 0.3f, 7, brighten(Color(0.2f, 0.25f, 0.3f), dim));
        Gfx::line(w / 2, -8, w / 2, -17, Color(0.8f, 0.8f, 0.85f), 1.5f);
        if (night > 0.3f) Gfx::circle(w / 2, -17, 2, Color(1, 0.2f, 0.2f, 0.5f + 0.5f * std::sin(time * 4)), 8);
        break; }
    case BK_HOSPITAL: {
        box(w, h, 9, brighten(Color(0.94f, 0.96f, 0.98f), dim), brighten(Color(0.80f, 0.86f, 0.92f), dim), brighten(Color(0.70f, 0.76f, 0.84f), dim));
        windows(6, 18, w - 12, h - 34, 2, 6, night, variant);
        Gfx::rect(w / 2 - 12, h - 14, 24, 14, brighten(Color(0.35f, 0.55f, 0.75f), dim));
        // red cross
        Gfx::rect(w / 2 - 3, 3, 6, 14, C_RED);
        Gfx::rect(w / 2 - 7, 7, 14, 6, C_RED);
        break; }
    case BK_SCHOOL: {
        box(w, h, 8, brighten(Color(0.95f, 0.74f, 0.42f), dim), brighten(Color(0.78f, 0.38f, 0.28f), dim), brighten(Color(0.78f, 0.55f, 0.30f), dim));
        windows(5, 12, w - 10, h - 26, 2, 6, night, variant);
        Gfx::rect(w / 2 - 8, h - 14, 16, 14, brighten(Color(0.45f, 0.28f, 0.18f), dim));
        Gfx::circle(w / 2, 6, 4.5f, Color(1, 1, 0.9f), 14);
        Gfx::line(w / 2, 6, w / 2, 3, Color(0.2f, 0.2f, 0.2f), 1);
        Gfx::line(w / 2, 6, w / 2 + 2, 6, Color(0.2f, 0.2f, 0.2f), 1);
        Gfx::line(w - 6, -8, w - 6, -26, Color(0.7f, 0.7f, 0.7f), 1.5f);          // flag pole
        Gfx::poly(std::vector<Vec2>{ Vec2(w - 6, -26), Vec2(w + 6 + 2 * std::sin(time * 3), -22), Vec2(w - 6, -18) }, C_RED);
        break; }
    case BK_MALL: {
        box(w, h, 8, brighten(Color(0.85f, 0.55f, 0.75f), dim), brighten(Color(0.70f, 0.40f, 0.60f), dim), brighten(Color(0.60f, 0.35f, 0.52f), dim));
        for (int i = 0; i < 8; i++)                                           // striped awning
            Gfx::rect(i * w / 8, h * 0.45f, w / 8, 8, (i % 2) ? Color(1, 1, 1) : Color(0.9f, 0.25f, 0.35f));
        windows(4, h * 0.45f + 10, w - 8, h * 0.45f - 10, 1, 7, night, variant);
        Gfx::textCentered(w / 2, 14, "SHOPPING MALL", Color(1, 1, 1), FONT_S);
        break; }
    case BK_RECYCLE: {
        box(w, h, 9, brighten(Color(0.30f, 0.62f, 0.42f), dim), brighten(Color(0.22f, 0.45f, 0.32f), dim), brighten(Color(0.18f, 0.40f, 0.28f), dim));
        // rotating recycling emblem (three arrows on a circle)
        float cx = w / 2, cy = h * 0.42f;
        Gfx::circle(cx, cy, 15, Color(0.95f, 1, 0.95f, 0.9f), 24);
        for (int k = 0; k < 3; k++) {
            float a = time * 1.2f + k * 2.0944f;
            Vec2 p(cx + 9 * std::cos(a), cy + 9 * std::sin(a));
            Vec2 q(cx + 9 * std::cos(a + 0.6f), cy + 9 * std::sin(a + 0.6f));
            Vec2 r(cx + 13.5f * std::cos(a + 0.3f), cy + 13.5f * std::sin(a + 0.3f));
            Gfx::triangle(p, q, r, C_DGREEN);
        }
        Gfx::rect(w * 0.1f, h - 12, w * 0.8f, 12, brighten(Color(0.2f, 0.25f, 0.28f), dim));       // conveyor
        for (int k = 0; k < 8; k++)
            Gfx::rect(w * 0.1f + std::fmod(k * 11 + time * 14, w * 0.8f - 4), h - 10, 4, 4, wasteInfo2Color(k));
        break; }
    default: { // BK_DEPOT
        box(w, h, 9, brighten(Color(0.55f, 0.60f, 0.66f), dim), brighten(Color(0.40f, 0.45f, 0.52f), dim), brighten(Color(0.38f, 0.42f, 0.48f), dim));
        Gfx::rect(0, 0, w, 14, brighten(Color(0.13f, 0.50f, 0.36f), dim));
        Gfx::textCentered(w / 2, 11, "COLLECTION CENTER", Color(1, 1, 1), FONT_S);
        for (int k = 0; k < 3; k++) {
            float bx = 6 + k * (w - 12) / 3;
            Gfx::rect(bx, 24, (w - 12) / 3 - 6, h - 24, brighten(Color(0.20f, 0.22f, 0.28f), dim));
            Gfx::rect(bx, 24, (w - 12) / 3 - 6, 6, Color(0.9f, 0.7f, 0.1f));
        }
        break; }
    }
}

void drawTree(float sway, int variant) {
    float s = sway * 1.5f;
    Gfx::ellipse(2, 1, 9, 3.5f, Color(0, 0, 0, 0.22f), 14);
    Gfx::rect(-1.5f, -8, 3, 9, Color(0.40f, 0.27f, 0.16f));
    Color g1 = (variant % 2) ? Color(0.16f, 0.52f, 0.25f) : Color(0.22f, 0.58f, 0.22f);
    Gfx::circle(-4 + s * 0.6f, -12, 7, brighten(g1, 0.85f), 16);
    Gfx::circle(4 + s * 0.6f, -12, 7, brighten(g1, 0.9f), 16);
    Gfx::circle(s, -17, 8, g1, 16);
    Gfx::circle(s - 2, -19, 3.5f, brighten(g1, 1.25f), 10);
}

void drawCar(const Color& c, float night) {
    Gfx::ellipse(0, 5, 10, 3, Color(0, 0, 0, 0.2f), 12);
    Gfx::roundRect(-9, -5, 18, 10, 3, c);
    Gfx::rect(1, -4, 5, 8, Color(0.75f, 0.88f, 0.98f));
    Gfx::rect(-6, -4, 3, 8, Color(0.55f, 0.70f, 0.80f));
    Gfx::circle(9, -3, 1.4f, mixColor(Color(0.95f, 0.95f, 0.8f), Color(1, 1, 0.6f), night), 6);
    Gfx::circle(9, 3, 1.4f, mixColor(Color(0.95f, 0.95f, 0.8f), Color(1, 1, 0.6f), night), 6);
}

static void wheel(float x, float y, float angle) {
    Gfx::circle(x, y, 4.6f, Color(0.08f, 0.08f, 0.10f), 14);
    Gfx::circle(x, y, 2.4f, Color(0.60f, 0.62f, 0.66f), 10);
    float a = angle * PI_F / 180.0f;
    for (int k = 0; k < 3; k++) {                     // rotating spokes
        float b = a + k * 2.0944f;
        Gfx::line(x, y, x + 2.4f * std::cos(b), y + 2.4f * std::sin(b), Color(0.15f, 0.15f, 0.18f), 1.2f);
    }
}

void drawTruck(const Color& body, float wheelAngle, float cargoFill, float night) {
    Gfx::ellipse(0, 3, 25, 12, Color(0, 0, 0, 0.25f), 20);
    wheel(-14, -10.5f, wheelAngle); wheel(-14, 10.5f, wheelAngle);
    wheel(12, -10.5f, wheelAngle);  wheel(12, 10.5f, wheelAngle);
    Gfx::roundRect(-23, -9, 30, 18, 3, body);                                  // cargo box
    Gfx::rect(-21, -7, 26, 3, brighten(body, 1.35f));
    Gfx::rect(-21, 4, 26, 3, brighten(body, 0.7f));
    // fill indicator on the roof of the box
    Gfx::rect(-19, -2, 22, 4, Color(0, 0, 0, 0.35f));
    Gfx::rect(-19, -2, 22 * clampf(cargoFill, 0, 1), 4, C_YELLOW);
    Gfx::roundRect(6, -8, 17, 16, 3, Color(0.92f, 0.94f, 0.96f));              // cab
    Gfx::rect(15, -6, 6, 12, Color(0.45f, 0.72f, 0.92f));                      // windscreen
    Gfx::circle(23, -5, 1.8f, mixColor(Color(0.95f, 0.95f, 0.8f), Color(1, 1, 0.5f), night), 6);
    Gfx::circle(23, 5, 1.8f, mixColor(Color(0.95f, 0.95f, 0.8f), Color(1, 1, 0.5f), night), 6);
    if (night > 0.2f) {                                                        // head-light cones
        Gfx::poly(std::vector<Vec2>{ Vec2(24, -5), Vec2(46, -12), Vec2(46, 12), Vec2(24, 5) }, Color(1, 1, 0.7f, 0.14f * night));
    }
}

void drawBin(const Color& levelColor, const Color& catColor, float fill01, bool selected,
             bool flashing, float lift, float time) {
    if (selected) {
        float p = 0.5f + 0.5f * std::sin(time * 6);
        Gfx::circleOutline(0, 0, 13 + 2 * p, Color(1, 1, 1, 0.9f), 2, 24);
        Gfx::circle(0, 0, 12, Color(1, 1, 1, 0.10f), 20);
    }
    if (flashing && std::fmod(time, 0.8f) < 0.4f) Gfx::circle(0, 0, 15, Color(1, 0.1f, 0.1f, 0.25f), 20);
    Gfx::ellipse(1, 7.5f, 7, 2.5f, Color(0, 0, 0, 0.3f), 12);
    float ly = -lift;                                                         // lid lifts while a truck collects
    std::vector<Vec2> body;
    body.push_back(Vec2(-5, -5)); body.push_back(Vec2(5, -5)); body.push_back(Vec2(4, 7)); body.push_back(Vec2(-4, 7));
    Gfx::poly(body, Color(0.20f, 0.26f, 0.32f));
    // fill gauge
    float gh = 11.0f * clampf(fill01, 0, 1);
    Gfx::rect(-3.2f, 6 - gh, 6.4f, gh, levelColor);
    Gfx::rect(-5, -1, 10, 2.2f, catColor);                                     // category band
    Gfx::roundRect(-6.5f, -8 + ly, 13, 3.5f, 1.5f, brighten(catColor, 0.9f));  // lid
    Gfx::rect(-1.5f, -10 + ly, 3, 2, Color(0.7f, 0.7f, 0.75f));
    Gfx::polyOutline(body, Color(0, 0, 0, 0.5f), 1, true);
}

} // namespace Objects
