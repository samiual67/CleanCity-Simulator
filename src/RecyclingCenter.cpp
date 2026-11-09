#include "RecyclingCenter.h"
#include "WasteManager.h"

void RecyclingCenter::reset() {
    for (int i = 0; i < W_COUNT; i++) received[i] = processed[i] = 0;
}
void RecyclingCenter::receive(const float cargo[W_COUNT]) {
    for (int i = 0; i < W_COUNT; i++) received[i] += cargo[i];
}
void RecyclingCenter::update(float dt) {
    for (int i = 0; i < W_COUNT; i++) {
        float room = recyclable(i) - processed[i];
        if (room > 0) processed[i] += std::min(room, 6.0f * dt);    // 6 kg/s per line
    }
}
float RecyclingCenter::recyclable(int t) const { return received[t] * wasteInfo(t).recycleRate; }
float RecyclingCenter::progress(int t) const {
    float r = recyclable(t);
    return r > 0.001f ? clampf(processed[t] / r, 0, 1) : 0;
}
float RecyclingCenter::totalReceived() const { float s = 0; for (int i = 0; i < W_COUNT; i++) s += received[i]; return s; }
float RecyclingCenter::totalProcessed() const { float s = 0; for (int i = 0; i < W_COUNT; i++) s += processed[i]; return s; }
float RecyclingCenter::totalRecyclable() const { float s = 0; for (int i = 0; i < W_COUNT; i++) s += recyclable(i); return s; }
