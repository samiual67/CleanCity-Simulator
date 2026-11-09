// Common.h - shared types, constants and the colour theme of CleanCity Simulator
#pragma once
#include <cmath>
#include <string>
#include <vector>
#include <algorithm>
#include <cstdio>

const int   WIN_W = 1280;   // logical window size (UI is laid out in these units)
const int   WIN_H = 720;
const float PI_F  = 3.14159265358979f;

struct Vec2 {
    float x, y;
    Vec2() : x(0), y(0) {}
    Vec2(float a, float b) : x(a), y(b) {}
    Vec2 operator+(const Vec2& o) const { return Vec2(x + o.x, y + o.y); }
    Vec2 operator-(const Vec2& o) const { return Vec2(x - o.x, y - o.y); }
    Vec2 operator*(float s) const { return Vec2(x * s, y * s); }
};

struct Color {
    float r, g, b, a;
    Color(float R = 0, float G = 0, float B = 0, float A = 1) : r(R), g(G), b(B), a(A) {}
};

inline float clampf(float v, float lo, float hi) { return v < lo ? lo : (v > hi ? hi : v); }
inline float lerpf(float a, float b, float t) { return a + (b - a) * t; }
inline float distf(const Vec2& a, const Vec2& b) { return std::sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y)); }
inline Color mixColor(const Color& a, const Color& b, float t) {
    return Color(lerpf(a.r, b.r, t), lerpf(a.g, b.g, t), lerpf(a.b, b.b, t), lerpf(a.a, b.a, t));
}
inline Color withAlpha(Color c, float a) { c.a = a; return c; }
inline Color brighten(const Color& c, float k) {
    return Color(clampf(c.r * k, 0, 1), clampf(c.g * k, 0, 1), clampf(c.b * k, 0, 1), c.a);
}

// ---- Theme: dark navy + deep green smart-city control centre ----
const Color C_BG       (0.035f, 0.060f, 0.105f);
const Color C_BG2      (0.050f, 0.100f, 0.140f);
const Color C_PANEL    (0.070f, 0.120f, 0.190f);
const Color C_PANEL2   (0.095f, 0.165f, 0.250f);
const Color C_BORDER   (0.18f, 0.30f, 0.40f);
const Color C_TEXT     (0.93f, 0.97f, 0.99f);
const Color C_DIM      (0.56f, 0.67f, 0.75f);
const Color C_GREEN    (0.17f, 0.80f, 0.45f);
const Color C_DGREEN   (0.05f, 0.35f, 0.25f);
const Color C_TEAL     (0.10f, 0.68f, 0.60f);
const Color C_YELLOW   (0.98f, 0.83f, 0.20f);
const Color C_ORANGE   (0.98f, 0.55f, 0.15f);
const Color C_RED      (0.92f, 0.26f, 0.26f);
const Color C_BLUE     (0.25f, 0.56f, 0.95f);

// Waste categories
enum WasteType { W_ORGANIC = 0, W_PLASTIC, W_PAPER, W_METAL, W_GLASS, W_COUNT };
