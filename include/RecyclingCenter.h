// RecyclingCenter.h - receives waste from trucks and processes the recyclable part over time
#pragma once
#include "Common.h"

class RecyclingCenter {
public:
    float received[W_COUNT];     // kg delivered by trucks
    float processed[W_COUNT];    // kg already recycled

    RecyclingCenter() { reset(); }
    void  reset();
    void  receive(const float cargo[W_COUNT]);
    void  update(float dt);
    float recyclable(int type) const;           // received * recycle rate
    float progress(int type) const;             // processed / recyclable (0..1)
    float totalReceived() const;
    float totalProcessed() const;
    float totalRecyclable() const;
};
