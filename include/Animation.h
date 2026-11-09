// Animation.h - time based animation helpers
#pragma once
#include "Common.h"

// Moves an object along a list of way-points at constant speed (time based, not frame based)
struct Mover {
    Vec2  pos;
    float angle;                 // heading in degrees (0 = east, y axis points down)
    float speed;                 // world units per second
    bool  active;
    std::vector<Vec2> wp;
    size_t ti;                   // index of way-point currently driven to

    Mover() : angle(0), speed(100), active(false), ti(0) {}
    void setPath(const std::vector<Vec2>& path);
    bool update(float dt);       // returns true on the frame the final way-point is reached
};

// Exponentially smoothed value for animated dashboard numbers
struct SmoothValue {
    float cur, target;
    SmoothValue() : cur(0), target(0) {}
    void update(float dt, float rate = 5.0f) {
        cur += (target - cur) * clampf(dt * rate, 0.0f, 1.0f);
        if (std::fabs(target - cur) < 0.01f) cur = target;
    }
};

float easeInOut(float t);
float pulse(float time, float speed);     // 0..1 sine pulse
