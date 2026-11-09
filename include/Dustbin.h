// Dustbin.h - a virtual smart dustbin (pure data + small helpers, no rendering)
#pragma once
#include "Common.h"

enum CollectionStatus { CS_WAITING = 0, CS_ASSIGNED, CS_COLLECTING };

struct Dustbin {
    int   id;
    Vec2  pos;               // world position (next to a road intersection)
    int   nodeId;            // nearest road graph node
    int   zone;
    float capacity;          // kg
    float level;             // kg currently inside
    int   category;          // WasteType
    float fillRate;          // kg per second while the simulation runs
    CollectionStatus status;
    bool  alerted;           // "bin full" warning already raised
    float flash;             // visual flash timer after warning

    Dustbin() : id(0), nodeId(0), zone(0), capacity(100), level(0), category(0),
                fillRate(1), status(CS_WAITING), alerted(false), flash(0) {}

    float percent() const { return capacity > 0 ? 100.0f * level / capacity : 0; }
    int   levelClass() const;            // 0 low, 1 medium, 2 high, 3 full
    Color levelColor() const;
    const char* levelName() const;
    const char* statusName() const;
    std::string label() const;           // "DB-03"
    void  addWaste(float kg);
    float empty();                       // returns removed kg
};
