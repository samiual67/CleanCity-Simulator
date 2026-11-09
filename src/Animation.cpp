#include "Animation.h"

void Mover::setPath(const std::vector<Vec2>& path) {
    wp = path;
    ti = 0;
    active = !wp.empty();
}

bool Mover::update(float dt) {
    if (!active) return false;
    float step = speed * dt;
    while (step > 0 && ti < wp.size()) {
        Vec2 target = wp[ti];
        float d = distf(pos, target);
        if (d > 0.001f) {
            float desired = std::atan2(target.y - pos.y, target.x - pos.x) * 180.0f / PI_F;
            float diff = desired - angle;
            while (diff > 180) diff -= 360;
            while (diff < -180) diff += 360;
            angle += diff * clampf(dt * 12.0f, 0.0f, 1.0f);   // smooth turning at intersections
        }
        if (d <= step) {
            pos = target;
            step -= d;
            ti++;
        } else {
            pos.x += (target.x - pos.x) / d * step;
            pos.y += (target.y - pos.y) / d * step;
            step = 0;
        }
    }
    if (ti >= wp.size()) { active = false; return true; }
    return false;
}

float easeInOut(float t) { t = clampf(t, 0, 1); return t * t * (3 - 2 * t); }
float pulse(float time, float speed) { return 0.5f + 0.5f * std::sin(time * speed); }
