// Simulation.h - the complete simulation state + user commands (no rendering code here)
#pragma once
#include "Common.h"
#include "RoutePlanner.h"
#include "WasteManager.h"
#include "RecyclingCenter.h"
#include "GarbageTruck.h"
#include "CityLayout.h"

struct AmbientCar {
    Mover mover;
    Color color;
    int   node, prevNode;
};

class Simulation {
public:
    RoutePlanner    planner;
    WasteManager    waste;
    RecyclingCenter recycling;
    std::vector<GarbageTruck> trucks;
    std::vector<AmbientCar>   cars;
    int   depotNode;

    bool  started, paused, traffic, autoDispatch;
    float dayTarget, dayFactor;         // 1 = day, 0 = night (dayFactor eases towards target)
    float time;                         // real time for ambient animation (clouds, water, lights)
    float speed;                        // simulation speed multiplier
    int   selBin, selTruck;

    // route preview ("Find Shortest Route")
    std::vector<int> previewRoute;
    float previewDist, previewCost;
    int   previewVisited;

    std::string toast;
    float toastTimer;

    Simulation();
    void init();
    void reset();
    void update(float dt);
    bool running() const { return started && !paused; }
    int  activeTrucks() const;

    // user commands (all return a message through the toast)
    void cmdStart();
    void cmdPause();
    void cmdResume();
    void cmdAddWaste();
    void cmdEmptySelected();
    void cmdDispatch();
    void cmdFindRoute();
    void cmdToggleTraffic();
    void cmdToggleDayNight();
    void cmdCycleCategory();
    void cmdToggleAuto();
    void cmdCycleSpeed();

    void say(const std::string& s) { toast = s; toastTimer = 3.5f; }

private:
    float autoTimer;
    void applyTraffic();
    bool dispatchPair(int truckId, int binId);
    int  pickIdleTruck() const;
    int  pickBestBin(float minPercent) const;
    void updateCars(float dt);
};
