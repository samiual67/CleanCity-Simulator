#include "Dustbin.h"

int Dustbin::levelClass() const {
    float p = percent();
    if (p >= 99.99f) return 3;       // 100%  -> full
    if (p > 70.0f)   return 2;       // 71-99 -> high
    if (p > 30.0f)   return 1;       // 31-70 -> medium
    return 0;                        // 0-30  -> low
}
Color Dustbin::levelColor() const {
    switch (levelClass()) {
        case 0: return C_GREEN;
        case 1: return C_YELLOW;
        case 2: return C_ORANGE;
        default: return C_RED;
    }
}
const char* Dustbin::levelName() const {
    static const char* n[] = { "Low", "Medium", "High", "Full" };
    return n[levelClass()];
}
const char* Dustbin::statusName() const {
    switch (status) {
        case CS_ASSIGNED:   return "Truck assigned";
        case CS_COLLECTING: return "Being collected";
        default:            return "Waiting";
    }
}
std::string Dustbin::label() const {
    char b[16];
    std::snprintf(b, sizeof(b), "DB-%02d", id + 1);
    return b;
}
void Dustbin::addWaste(float kg) {
    level = clampf(level + kg, 0, capacity);
}
float Dustbin::empty() {
    float r = level;
    level = 0;
    alerted = false;
    return r;
}
