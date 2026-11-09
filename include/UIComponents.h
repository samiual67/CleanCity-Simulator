// UIComponents.h - reusable UI widgets (panels, buttons, cards, charts) drawn with Gfx
#pragma once
#include "Common.h"
#include "Gfx.h"

struct Button {
    int  id;
    float x, y, w, h;
    std::string label;
    bool enabled;
    bool toggled;
    Color accent;
    Button() : id(0), x(0), y(0), w(100), h(32), enabled(true), toggled(false), accent(C_TEAL) {}
    Button(int i, float X, float Y, float W, float H, const std::string& l, const Color& a = C_TEAL)
        : id(i), x(X), y(Y), w(W), h(H), label(l), enabled(true), toggled(false), accent(a) {}
    bool contains(float px, float py) const { return px >= x && px <= x + w && py >= y && py <= y + h; }
};

namespace UI {
    void background(float time);                                           // animated navy gradient
    void panel(float x, float y, float w, float h, const char* title = 0);
    void button(const Button& b, bool hover, FontSize f = FONT_S);
    void statCard(float x, float y, float w, float h, const std::string& title, const std::string& value,
                  const std::string& sub, const Color& accent, float time);
    void progressBar(float x, float y, float w, float h, float frac, const Color& c);
    void barChart(float x, float y, float w, float h, const std::vector<std::string>& labels,
                  const std::vector<float>& values, const std::vector<Color>& colors, float maxValue);
    void pieChart(float cx, float cy, float r, const std::vector<float>& values, const std::vector<Color>& colors);
    void gauge(float cx, float cy, float r, float frac, const Color& c);
    void wasteIcon(float cx, float cy, float size, int type);              // a distinct icon per waste category
    void logo(float cx, float cy, float r, float time);
}
