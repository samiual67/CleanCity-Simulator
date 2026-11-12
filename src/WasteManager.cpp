#include "WasteManager.h"

static const WasteInfo kInfo[W_COUNT] = {
    { "Organic", Color(0.40f, 0.78f, 0.30f), 0.85f, "Food & garden waste. Composted into soil enrichment and biogas." },
    { "Plastic", Color(0.25f, 0.60f, 0.95f), 0.60f, "Bottles & packaging. Shredded, melted and re-moulded into pellets." },
    { "Paper",   Color(0.95f, 0.80f, 0.35f), 0.80f, "Paper & cardboard. Pulped, de-inked and pressed into new sheets." },
    { "Metal",   Color(0.68f, 0.72f, 0.80f), 0.95f, "Cans & scrap. Sorted magnetically, melted and cast again." },
    { "Glass",   Color(0.30f, 0.85f, 0.80f), 0.90f, "Bottles & jars. Crushed into cullet and melted - endlessly recyclable." }
};
const WasteInfo& wasteInfo(int t) { return kInfo[t < 0 ? 0 : (t >= W_COUNT ? W_COUNT - 1 : t)]; }

const char* WasteManager::zoneName(int z) {
    static const char* n[ZONES] = { "Green Park", "Downtown", "Riverside", "Industrial Hub" };
    return n[z];
}

void WasteManager::reset() {
    bins.clear();
    history.clear();
    log.clear();
    for (int i = 0; i < W_COUNT; i++) collected[i] = 0;
    totalCollected = 0;
    collectionsDone = 0;
    clock = 0;
}

void WasteManager::init(const RoutePlanner& rp) {
    reset();
    // predefined sample data: node, capacity (kg), initial level (%), waste category
    struct Sample { int node; float cap; float pct; int cat; };
    static const Sample S[] = {
        { 0, 100, 85, W_ORGANIC }, { 2, 120, 40, W_PLASTIC }, { 4,  80, 15, W_PAPER },
        { 6, 150, 62, W_METAL },   { 8, 100, 95, W_GLASS },   { 10, 120, 30, W_ORGANIC },
        { 12, 90, 72, W_PLASTIC }, { 13, 100, 10, W_PAPER },  { 14, 140, 55, W_METAL },
        { 16, 100, 100, W_GLASS }, { 17, 110, 25, W_ORGANIC }, { 18, 100, 48, W_PLASTIC }
    };
    static const Vec2 OFF[4] = { Vec2(24, -24), Vec2(-24, -24), Vec2(24, 24), Vec2(-24, 24) };
    const int n = (int)(sizeof(S) / sizeof(S[0]));
    for (int i = 0; i < n; i++) {
        Dustbin b;
        b.id = i;
        b.nodeId = S[i].node;
        b.pos = rp.nodes()[S[i].node] + OFF[i % 4];
        b.capacity = S[i].cap;
        b.level = S[i].cap * S[i].pct / 100.0f;
        b.category = S[i].cat;
        b.fillRate = S[i].cap / (110.0f + 17.0f * (i % 6));       // full in ~2-3.5 minutes
        b.zone = (b.pos.x < 500 ? 0 : 1) + (b.pos.y < 300 ? 0 : 2);
        // zone ids: 0 NW, 1 NE, 2 SW, 3 SE
        b.alerted = (b.percent() >= 99.99f);
        bins.push_back(b);
    }
    addLog("System ready - 12 smart dustbins online");
}

void WasteManager::addLog(const std::string& s) {
    int sec = (int)clock;
    char t[24];
    std::snprintf(t, sizeof(t), "[%02d:%02d] ", sec / 60, sec % 60);
    log.push_back(std::string(t) + s);
    if (log.size() > 40) log.erase(log.begin());
}

void WasteManager::update(float dt) {
    clock += dt;
    for (size_t i = 0; i < bins.size(); i++) {
        Dustbin& b = bins[i];
        if (b.status != CS_COLLECTING) b.addWaste(b.fillRate * dt);
        if (b.flash > 0) b.flash -= dt;
        if (b.percent() >= 99.99f && !b.alerted) {
            b.alerted = true;
            b.flash = 3.0f;
            addLog("WARNING: " + b.label() + " is FULL (" + zoneName(b.zone) + ")");
        }
    }
}

void WasteManager::recordCollection(int truckId, const Dustbin& b, float amount) {
    collected[b.category] += amount;
    totalCollected += amount;
    collectionsDone++;
    CollectionRecord r;
    r.truckId = truckId; r.binId = b.id; r.category = b.category; r.amount = amount; r.time = clock;
    history.push_back(r);
}

void WasteManager::addWaste(int id, float f) {
    if (id < 0 || id >= (int)bins.size()) return;
    bins[id].addWaste(bins[id].capacity * f);
}
void WasteManager::addWasteAll(float f) {
    for (size_t i = 0; i < bins.size(); i++)
        if (bins[i].status != CS_COLLECTING) bins[i].addWaste(bins[i].capacity * f);
}

int WasteManager::countFull() const {
    int n = 0; for (size_t i = 0; i < bins.size(); i++) if (bins[i].levelClass() == 3) n++; return n;
}
int WasteManager::countEmpty() const {
    int n = 0; for (size_t i = 0; i < bins.size(); i++) if (bins[i].percent() < 5.0f) n++; return n;
}
int WasteManager::countPartial() const {
    return (int)bins.size() - countFull() - countEmpty();
}
float WasteManager::totalRemaining() const {
    float s = 0; for (size_t i = 0; i < bins.size(); i++) s += bins[i].level; return s;
}
float WasteManager::remainingByCategory(int c) const {
    float s = 0; for (size_t i = 0; i < bins.size(); i++) if (bins[i].category == c) s += bins[i].level; return s;
}

ZoneStat WasteManager::zone(int z) const {
    ZoneStat s;
    s.name = zoneName(z);
    s.bins = 0; s.fullBins = 0; s.waste = 0; s.capacity = 0; s.collectionPending = false;
    for (size_t i = 0; i < bins.size(); i++) {
        const Dustbin& b = bins[i];
        if (b.zone != z) continue;
        s.bins++; s.waste += b.level; s.capacity += b.capacity;
        if (b.levelClass() == 3) s.fullBins++;
        if (b.status != CS_WAITING) s.collectionPending = true;
    }
    s.cleanliness = s.capacity > 0 ? 100.0f * (1.0f - s.waste / s.capacity) : 100.0f;
    if (s.cleanliness >= 60)      { s.color = C_GREEN;  s.status = "Clean"; }
    else if (s.cleanliness >= 35) { s.color = C_YELLOW; s.status = "Needs attention"; }
    else                          { s.color = C_RED;    s.status = "Critical"; }
    return s;
}

float WasteManager::cityCleanliness() const {
    float w = 0, c = 0;
    for (size_t i = 0; i < bins.size(); i++) { w += bins[i].level; c += bins[i].capacity; }
    return c > 0 ? 100.0f * (1.0f - w / c) : 100.0f;
}
