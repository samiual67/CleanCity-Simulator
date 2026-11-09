// WasteManager.h - owns the dustbins, waste statistics, zones and the activity log
#pragma once
#include "Common.h"
#include "Dustbin.h"
#include "RoutePlanner.h"

struct WasteInfo {
    const char* name;
    Color       color;
    float       recycleRate;     // fraction that can be recycled
    const char* info;            // recycling information shown in the Recycling Centre
};
const WasteInfo& wasteInfo(int type);

struct ZoneStat {
    std::string name;
    int   bins, fullBins;
    float waste, capacity;
    float cleanliness;           // 0..100
    Color color;
    const char* status;          // Clean / Needs attention / Critical
    bool  collectionPending;
};

struct CollectionRecord {
    int truckId, binId, category;
    float amount, time;
};

class WasteManager {
public:
    static const int ZONES = 4;
    std::vector<Dustbin> bins;
    float collected[W_COUNT];
    float totalCollected;
    int   collectionsDone;
    std::vector<CollectionRecord> history;
    std::vector<std::string> log;       // newest last
    float clock;                        // simulation clock in seconds

    WasteManager() { reset(); }
    void  init(const RoutePlanner& rp);           // places the sample bins (predefined data)
    void  reset();
    void  update(float dt);                       // bins fill up over time
    void  addLog(const std::string& s);

    void  recordCollection(int truckId, const Dustbin& b, float amount);
    void  addWaste(int binId, float fractionOfCapacity);
    void  addWasteAll(float fractionOfCapacity);

    int   countFull() const;
    int   countEmpty() const;
    int   countPartial() const;
    float totalRemaining() const;
    float remainingByCategory(int c) const;
    ZoneStat zone(int z) const;
    float cityCleanliness() const;
    static const char* zoneName(int z);
};
