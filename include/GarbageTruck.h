// GarbageTruck.h - garbage truck state machine (logic only)
#pragma once
#include "Common.h"
#include "Animation.h"
#include "RoutePlanner.h"
#include "WasteManager.h"
#include "RecyclingCenter.h"

enum TruckState { T_IDLE = 0, T_TO_BIN, T_COLLECTING, T_RETURNING, T_UNLOADING };
static const float LANE_OFFSET = 11.5f;   // trucks drive in the left lane of the road

struct GarbageTruck {
    int   id;
    Vec2  home;                      // parking spot at the collection center
    Mover mover;
    TruckState state;
    int   targetBin;
    std::vector<int> route;          // road node ids of the current route (for highlighting)
    float routeDist, routeCost;
    float timer, progress;           // collection / unloading progress
    float cargo[W_COUNT];
    float wheelAngle;
    int   collections;
    float totalCollected;
    Color color;

    GarbageTruck() : id(0), state(T_IDLE), targetBin(-1), routeDist(0), routeCost(0),
                     timer(0), progress(0), wheelAngle(0), collections(0), totalCollected(0) {
        for (int i = 0; i < W_COUNT; i++) cargo[i] = 0;
    }

    void init(int truckId, Vec2 homePos, Color c);
    bool isIdle() const { return state == T_IDLE; }
    // Plans the Dijkstra route depot -> bin and starts driving. Returns false if impossible.
    bool assign(int binId, const RoutePlanner& rp, WasteManager& wm, int depotNode);
    void update(float dt, const RoutePlanner& rp, WasteManager& wm, RecyclingCenter& rc, int depotNode);
    std::string label() const;
    std::string statusText() const;
    float cargoTotal() const;
};
