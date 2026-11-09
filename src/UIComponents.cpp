#include "UIComponents.h"
#include "WasteManager.h"

namespace UI {

void background(float time) {
    Gfx::rectGradient(0, 0, WIN_W, WIN_H, C_BG2, C_BG);
    // faint moving grid, gives a "control room" feel
    float off = std::fmod(time * 6.0f, 40.0f);
    for (float x = -40 + off; x < WIN_W; x += 40) Gfx::line(x, 0, x, WIN_H, Color(0.2f, 0.4f, 0.5f, 0.05f), 1);
    for (float y = 0; y < WIN_H; y += 40) Gfx::line(0, y, WIN_W, y, Color(0.2f, 0.4f, 0.5f, 0.05f), 1);
}

void panel(float x, float y, float w, float h, const char* title) {
    Gfx::roundRect(x + 2, y + 3, w, h, 10, Color(0, 0, 0, 0.30f));
    Gfx::roundRectGradient(x, y, w, h, 10, C_PANEL2, C_PANEL);
    Gfx::roundRectOutline(x, y, w, h, 10, C_BORDER, 1.2f);
    if (title) {
        Gfx::text(x + 14, y + 22, title, C_TEXT, FONT_M);
        Gfx::rectGradientH(x + 14, y + 29, w - 28, 2, withAlpha(C_GREEN, 0.9f), withAlpha(C_GREEN, 0.0f));
    }
}

void button(const Button& b, bool hover, FontSize f) {
    Color base = b.toggled ? b.accent : C_PANEL2;
    Color top = brighten(base, hover && b.enabled ? 1.35f : 1.12f);
    Color bot = brighten(base, hover && b.enabled ? 1.0f : 0.78f);
    if (!b.enabled) { top = Color(0.14f, 0.18f, 0.22f); bot = Color(0.10f, 0.13f, 0.17f); }
    Gfx::roundRect(b.x + 1, b.y + 2, b.w, b.h, 8, Color(0, 0, 0, 0.35f));
    Gfx::roundRectGradient(b.x, b.y, b.w, b.h, 8, top, bot);
    Gfx::roundRectOutline(b.x, b.y, b.w, b.h, 8, b.enabled ? (hover ? b.accent : brighten(b.accent, 0.7f)) : C_BORDER, hover ? 2.0f : 1.2f);
    Gfx::rectGradient(b.x + 3, b.y + 2, b.w - 6, b.h * 0.35f, Color(1, 1, 1, 0.10f), Color(1, 1, 1, 0.0f));
    int tw = Gfx::textWidth(b.label, f);
    float ty = b.y + b.h / 2 + (f == FONT_S ? 4.5f : (f == FONT_M ? 6.0f : 8.0f));
    Gfx::text(b.x + b.w / 2 - tw / 2.0f, ty, b.label, b.enabled ? C_TEXT : C_DIM, f);
}

void statCard(float x, float y, float w, float h, const std::string& title, const std::string& value,
              const std::string& sub, const Color& accent, float time) {
    Gfx::roundRect(x + 2, y + 3, w, h, 10, Color(0, 0, 0, 0.30f));
    Gfx::roundRectGradient(x, y, w, h, 10, C_PANEL2, C_PANEL);
    Gfx::roundRectOutline(x, y, w, h, 10, withAlpha(accent, 0.55f), 1.4f);
    Gfx::roundRect(x, y + 10, 5, h - 20, 2, accent);                          // accent strip
    float glow = 0.5f + 0.5f * std::sin(time * 2.0f + x * 0.01f);
    Gfx::circle(x + w - 28, y + 28, 12 + 2 * glow, withAlpha(accent, 0.12f + 0.08f * glow), 20);
    Gfx::circle(x + w - 28, y + 28, 5, accent, 12);
    Gfx::text(x + 18, y + 24, title, C_DIM);
    Gfx::text(x + 18, y + 62, value, C_TEXT, FONT_L);
    Gfx::text(x + 18, y + h - 10, sub, withAlpha(accent, 0.95f));
}

void progressBar(float x, float y, float w, float h, float frac, const Color& c) {
    frac = clampf(frac, 0, 1);
    Gfx::roundRect(x, y, w, h, h / 2, Color(0, 0, 0, 0.40f));
    if (frac > 0.001f) {
        float fw = std::max(h, w * frac);
        Gfx::roundRectGradient(x, y, fw, h, h / 2, brighten(c, 1.25f), brighten(c, 0.8f));
    }
}

void barChart(float x, float y, float w, float h, const std::vector<std::string>& labels,
              const std::vector<float>& values, const std::vector<Color>& colors, float maxValue) {
    if (maxValue < 1) maxValue = 1;
    for (int i = 0; i <= 4; i++) {                                           // grid
        float gy = y + h - h * i / 4.0f;
        Gfx::line(x, gy, x + w, gy, Color(1, 1, 1, 0.08f), 1);
        char b[16]; std::snprintf(b, sizeof(b), "%.0f", maxValue * i / 4.0f);
        Gfx::text(x - 4 - Gfx::textWidth(b), gy + 4, b, C_DIM);
    }
    size_t n = values.size();
    float slot = w / (float)n, bw = slot * 0.55f;
    for (size_t i = 0; i < n; i++) {
        float bh = h * clampf(values[i] / maxValue, 0, 1);
        float bx = x + slot * i + (slot - bw) / 2;
        Gfx::roundRectGradient(bx, y + h - bh, bw, std::max(bh, 2.0f), 4, brighten(colors[i], 1.25f), brighten(colors[i], 0.75f));
        char b[16]; std::snprintf(b, sizeof(b), "%.0f", values[i]);
        Gfx::textCentered(bx + bw / 2, y + h - bh - 6, b, C_TEXT);
        Gfx::textCentered(bx + bw / 2, y + h + 16, labels[i], C_DIM);
    }
}

void pieChart(float cx, float cy, float r, const std::vector<float>& values, const std::vector<Color>& colors) {
    float total = 0;
    for (size_t i = 0; i < values.size(); i++) total += values[i];
    Gfx::circle(cx + 3, cy + 4, r, Color(0, 0, 0, 0.3f), 40);
    if (total < 0.001f) { Gfx::circle(cx, cy, r, Color(0.15f, 0.22f, 0.30f), 40); return; }
    float a = -90;
    for (size_t i = 0; i < values.size(); i++) {
        float sweep = 360.0f * values[i] / total;
        if (sweep > 0.1f) Gfx::pie(cx, cy, r, a, a + sweep, colors[i]);
        a += sweep;
    }
    a = -90;
    for (size_t i = 0; i < values.size(); i++) {                             // separators
        float rad = a * PI_F / 180.0f;
        Gfx::line(cx, cy, cx + r * std::cos(rad), cy + r * std::sin(rad), C_PANEL, 2);
        a += 360.0f * values[i] / total;
    }
    Gfx::circle(cx, cy, r * 0.52f, C_PANEL, 40);                            // donut hole
}

void gauge(float cx, float cy, float r, float frac, const Color& c) {
    frac = clampf(frac, 0, 1);
    Gfx::arc(cx, cy, r - 14, r, 135, 405, Color(0, 0, 0, 0.4f));
    if (frac > 0.005f) Gfx::arc(cx, cy, r - 14, r, 135, 135 + 270 * frac, c);
    float a = (135 + 270 * frac) * PI_F / 180.0f;
    Gfx::circle(cx + (r - 7) * std::cos(a), cy + (r - 7) * std::sin(a), 6, Color(1, 1, 1), 12);
    char b[16]; std::snprintf(b, sizeof(b), "%d%%", (int)(frac * 100 + 0.5f));
    Gfx::textCentered(cx, cy + 10, b, C_TEXT, FONT_L);
}

void wasteIcon(float cx, float cy, float s, int type) {
    Color c = wasteInfo(type).color;
    Gfx::circle(cx, cy, s * 0.62f, withAlpha(c, 0.16f), 24);
    switch (type) {
    case W_ORGANIC: {                                                        // leaf
        std::vector<Vec2> p;
        p.push_back(Vec2(cx - s * 0.35f, cy + s * 0.30f)); p.push_back(Vec2(cx - s * 0.20f, cy - s * 0.15f));
        p.push_back(Vec2(cx + s * 0.10f, cy - s * 0.38f)); p.push_back(Vec2(cx + s * 0.38f, cy - s * 0.30f));
        p.push_back(Vec2(cx + s * 0.30f, cy + s * 0.05f)); p.push_back(Vec2(cx, cy + s * 0.30f));
        Gfx::poly(p, c);
        Gfx::line(cx - s * 0.35f, cy + s * 0.32f, cx + s * 0.25f, cy - s * 0.25f, C_PANEL, 1.5f);
        break; }
    case W_PLASTIC:                                                          // bottle
        Gfx::roundRect(cx - s * 0.17f, cy - s * 0.10f, s * 0.34f, s * 0.48f, 4, c);
        Gfx::rect(cx - s * 0.08f, cy - s * 0.32f, s * 0.16f, s * 0.24f, brighten(c, 0.8f));
        Gfx::rect(cx - s * 0.10f, cy - s * 0.38f, s * 0.20f, s * 0.07f, Color(1, 1, 1, 0.9f));
        Gfx::rect(cx - s * 0.17f, cy + s * 0.07f, s * 0.34f, s * 0.10f, Color(1, 1, 1, 0.5f));
        break;
    case W_PAPER:                                                            // sheet with folded corner
        Gfx::rect(cx - s * 0.25f, cy - s * 0.34f, s * 0.50f, s * 0.68f, c);
        Gfx::triangle(Vec2(cx + s * 0.25f, cy - s * 0.34f), Vec2(cx + s * 0.25f, cy - s * 0.14f), Vec2(cx + s * 0.05f, cy - s * 0.34f), Color(1, 1, 1, 0.8f));
        for (int i = 0; i < 4; i++) Gfx::line(cx - s * 0.16f, cy - s * 0.02f + i * s * 0.11f, cx + s * 0.16f, cy - s * 0.02f + i * s * 0.11f, C_PANEL, 1.5f);
        break;
    case W_METAL:                                                            // can
        Gfx::rect(cx - s * 0.20f, cy - s * 0.26f, s * 0.40f, s * 0.54f, c);
        Gfx::ellipse(cx, cy - s * 0.26f, s * 0.20f, s * 0.07f, brighten(c, 1.3f), 14);
        Gfx::ellipse(cx, cy + s * 0.28f, s * 0.20f, s * 0.07f, brighten(c, 0.7f), 14);
        Gfx::rect(cx - s * 0.20f, cy - s * 0.04f, s * 0.40f, s * 0.12f, C_BLUE);
        break;
    default: {                                                               // glass (gem / jar)
        std::vector<Vec2> p;
        p.push_back(Vec2(cx - s * 0.28f, cy - s * 0.10f)); p.push_back(Vec2(cx - s * 0.14f, cy - s * 0.34f));
        p.push_back(Vec2(cx + s * 0.14f, cy - s * 0.34f)); p.push_back(Vec2(cx + s * 0.28f, cy - s * 0.10f));
        p.push_back(Vec2(cx, cy + s * 0.36f));
        Gfx::poly(p, c);
        Gfx::line(cx - s * 0.28f, cy - s * 0.10f, cx + s * 0.28f, cy - s * 0.10f, Color(1, 1, 1, 0.7f), 1.2f);
        Gfx::line(cx - s * 0.08f, cy - s * 0.34f, cx - s * 0.02f, cy + s * 0.30f, Color(1, 1, 1, 0.5f), 1.2f);
        break; }
    }
}

void logo(float cx, float cy, float r, float time) {
    Gfx::circle(cx, cy, r * 1.25f, Color(0.17f, 0.80f, 0.45f, 0.10f), 40);
    Gfx::circle(cx, cy, r, C_DGREEN, 40);
    Gfx::circleOutline(cx, cy, r, C_GREEN, 3, 48);
    for (int k = 0; k < 3; k++) {                                            // rotating recycling arrows
        float a = time * 1.1f + k * 2.0944f;
        Vec2 p(cx + r * 0.62f * std::cos(a), cy + r * 0.62f * std::sin(a));
        Vec2 q(cx + r * 0.62f * std::cos(a + 0.9f), cy + r * 0.62f * std::sin(a + 0.9f));
        Vec2 t(cx + r * 0.88f * std::cos(a + 0.45f), cy + r * 0.88f * std::sin(a + 0.45f));
        Gfx::triangle(p, q, t, C_GREEN);
        Gfx::arc(cx, cy, r * 0.55f, r * 0.64f, a * 180 / PI_F + 5, a * 180 / PI_F + 40, C_GREEN);
    }
    Gfx::circle(cx, cy, r * 0.22f, Color(0.95f, 1, 0.95f), 20);
    Gfx::textCentered(cx, cy + 5, "CC", C_DGREEN, FONT_S);
}

} // namespace UI
