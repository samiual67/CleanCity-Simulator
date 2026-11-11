#include "Simulation.h"

Simulation::Simulation()
    : depotNode(0), started(false), paused(false), traffic(false), autoDispatch(false),
      dayTarget(1), dayFactor(1), time(0), speed(1), selBin(-1), selTruck(-1),
      previewDist(0), previewCost(0), previewVisited(0), toastTimer(0), autoTimer(0) {}

void Simulation::init() {
    City::buildGraph(planner, depotNode);
    waste.init(planner);
    recycling.reset();
    applyTraffic();

    trucks.clear();
    Vec2 d = planner.nodes()[depotNode];
    static const Color cols[3] = { Color(0.16f, 0.72f, 0.42f), Color(0.20f, 0.60f, 0.90f), Color(0.95f, 0.62f, 0.15f) };
    for (int i = 0; i < 3; i++) {
        GarbageTruck t;
        t.init(i, Vec2(d.x - 52 + 52 * i, d.y - 29), cols[i]);
        trucks.push_back(t);
    }

    cars.clear();
    static const Color carCols[6] = { Color(0.9f, 0.3f, 0.3f), Color(0.9f, 0.85f, 0.3f), Color(0.4f, 0.7f, 0.95f),
                                      Color(0.85f, 0.85f, 0.9f), Color(0.7f, 0.4f, 0.9f), Color(0.3f, 0.8f, 0.7f) };
    for (int i = 0; i < 6; i++) {
        AmbientCar c;
        c.node = (i * 3 + 1) % (City::COLS * City::ROWS);
        c.prevNode = -1;
        c.color = carCols[i];
        c.mover.pos = planner.nodes()[c.node];
        c.mover.speed = 55.0f + 9.0f * i;
        cars.push_back(c);
    }
    selBin = selTruck = -1;
    previewRoute.clear();
    started = false; paused = false;
    autoTimer = 0;
}

void Simulation::reset() {
    bool tr = traffic, au = autoDispatch;
    init();
    traffic = tr; autoDispatch = au;
    applyTraffic();
    waste.addLog("Simulation reset");
    say("Simulation reset");
}

int Simulation::activeTrucks() const {
    int n = 0;
    for (size_t i = 0; i < trucks.size(); i++) if (!trucks[i].isIdle()) n++;
    return n;
}

void Simulation::applyTraffic() {
    for (size_t e = 0; e < planner.edges().size(); e++)
        planner.setEdgeFactor((int)e, (traffic && City::isCongested((int)e)) ? 2.5f : 1.0f);
}

// true when two truck-sized boxes (centre, unit direction, half length, shared half width) overlap
static bool boxHit(const Vec2& c1, const Vec2& u1, float hl1, const Vec2& c2, const Vec2& u2, float hl2, float hw) {
    Vec2 v1(-u1.y, u1.x), v2(-u2.y, u2.x);
    Vec2 axes[4] = { u1, v1, u2, v2 };
    Vec2 d = c2 - c1;
    for (int k = 0; k < 4; k++) {
        const Vec2& ax = axes[k];
        float dist = std::fabs(d.x * ax.x + d.y * ax.y);
        float r1 = hl1 * std::fabs(u1.x * ax.x + u1.y * ax.y) + hw * std::fabs(v1.x * ax.x + v1.y * ax.y);
        float r2 = hl2 * std::fabs(u2.x * ax.x + u2.y * ax.y) + hw * std::fabs(v2.x * ax.x + v2.y * ax.y);
        if (dist >= r1 + r2) return false;
    }
    return true;
}

void Simulation::update(float dt) {
    time += dt;
    dayFactor += (dayTarget - dayFactor) * clampf(dt * 2.0f, 0, 1);
    if (std::fabs(dayTarget - dayFactor) < 0.002f) dayFactor = dayTarget;
    if (toastTimer > 0) toastTimer -= dt;

    if (!running()) return;
    float sdt = dt * speed;
    waste.update(sdt);
    static std::vector<int> waitFor;      // which truck each truck is waiting for (-1 = none)
    static std::vector<float> waitTime;
    if (waitFor.size() != trucks.size()) { waitFor.assign(trucks.size(), -1); waitTime.assign(trucks.size(), 0.0f); }
    for (size_t i = 0; i < trucks.size(); i++) {
        GarbageTruck& me = trucks[i];
        int wf = -1;
        if (me.mover.active && (me.state == T_TO_BIN || me.state == T_RETURNING)) {
            Vec2 tgt = me.mover.ti < me.mover.wp.size() ? me.mover.wp[me.mover.ti] : me.mover.pos;
            Vec2 dv = tgt - me.mover.pos;
            float dl = std::sqrt(dv.x * dv.x + dv.y * dv.y);
            if (dl > 0.01f) {
                Vec2 fwd = dv * (1.0f / dl);                              // direction of travel
                float ext = 8.0f + me.mover.speed * sdt * 2.0f;           // safety gap in front of the truck
                Vec2 probe = me.mover.pos + fwd * (ext * 0.5f);
                for (size_t j = 0; j < trucks.size(); j++) {
                    if (j == i) continue;
                    const GarbageTruck& o = trucks[j];
                    float oa = o.mover.angle * PI_F / 180.0f;
                    bool moving = o.mover.active && (o.state == T_TO_BIN || o.state == T_RETURNING);
                    bool stopped = !moving || waitFor[j] != -1;
                    if (waitFor[j] == (int)i || !(o.id < me.id || stopped)) continue;
                    if (boxHit(probe, fwd, 24.0f + ext * 0.5f, o.mover.pos, Vec2(std::cos(oa), std::sin(oa)), 24.0f, 9.0f)) { wf = (int)j; break; }
                }
            }
        }
        if (wf >= 0) { waitTime[i] += sdt; if (waitTime[i] > 5.0f) wf = -1; }     // never wait forever
        else waitTime[i] = 0;
        waitFor[i] = wf;
        me.update(wf >= 0 ? 0.0f : sdt, planner, waste, recycling, depotNode);   // keep distance, never drive through another truck
    }
    recycling.update(sdt);
    if (traffic) updateCars(sdt);

    if (autoDispatch) {
        autoTimer += sdt;
        if (autoTimer > 1.5f) {
            autoTimer = 0;
            int t = pickIdleTruck(), b = pickBestBin(70.0f);
            if (t >= 0 && b >= 0) dispatchPair(t, b);
        }
    }
}

void Simulation::updateCars(float dt) {
    for (size_t i = 0; i < cars.size(); i++) {
        AmbientCar& c = cars[i];
        if (c.mover.update(dt) || (!c.mover.active)) {
            std::vector<int> nb = planner.neighbors(c.node);
            // avoid immediate U-turns when possible
            std::vector<int> ok;
            for (size_t k = 0; k < nb.size(); k++) if (nb[k] != c.prevNode && nb[k] != depotNode) ok.push_back(nb[k]);
            if (ok.empty()) ok = nb;
            int next = ok[(size_t)((c.node * 7 + (int)(time * 10) + (int)i * 3) % (int)ok.size())];
            c.prevNode = c.node;
            c.node = next;
            std::vector<Vec2> p;
            p.push_back(planner.nodes()[next]);
            c.mover.setPath(p);
        }
    }
}

// ---------------- commands ----------------
void Simulation::cmdStart() {
    if (!started) { started = true; paused = false; waste.addLog("Simulation started"); say("Simulation started"); }
    else say("Simulation already running");
}
void Simulation::cmdPause() {
    if (!started) { say("Start the simulation first"); return; }
    paused = true; say("Simulation paused");
}
void Simulation::cmdResume() {
    if (!started) { cmdStart(); return; }
    paused = false; say("Simulation resumed");
}

void Simulation::cmdAddWaste() {
    if (selBin >= 0) {
        waste.addWaste(selBin, 0.25f);
        waste.addLog("Added waste to " + waste.bins[selBin].label());
        say("Added 25% waste to " + waste.bins[selBin].label());
    } else {
        waste.addWasteAll(0.10f);
        waste.addLog("Added waste to all dustbins");
        say("Added 10% waste to all dustbins");
    }
}

void Simulation::cmdEmptySelected() {
    if (selBin < 0) { say("Select a dustbin first (click it on the map)"); return; }
    Dustbin& b = waste.bins[selBin];
    if (b.status != CS_WAITING) { say(b.label() + " is being served by a truck"); return; }
    if (b.percent() < 1.0f) { say(b.label() + " is already empty"); return; }
    float amount = b.empty();
    waste.recordCollection(-1, b, amount);                // manual collection
    float cargo[W_COUNT] = { 0, 0, 0, 0, 0 };
    cargo[b.category] = amount;
    recycling.receive(cargo);
    waste.addLog(b.label() + " emptied manually (" + std::to_string((int)amount) + " kg)");
    say(b.label() + " emptied");
}

int Simulation::pickIdleTruck() const {
    if (selTruck >= 0 && trucks[selTruck].isIdle()) return selTruck;
    for (size_t i = 0; i < trucks.size(); i++) if (trucks[i].isIdle()) return (int)i;
    return -1;
}

int Simulation::pickBestBin(float minPercent) const {
    int best = -1; float bp = minPercent;
    for (size_t i = 0; i < waste.bins.size(); i++) {
        const Dustbin& b = waste.bins[i];
        if (b.status != CS_WAITING) continue;
        if (b.percent() >= bp) { bp = b.percent(); best = (int)i; }
    }
    return best;
}

bool Simulation::dispatchPair(int truckId, int binId) {
    GarbageTruck& t = trucks[truckId];
    if (!t.assign(binId, planner, waste, depotNode)) return false;
    previewRoute = t.route;
    previewDist = t.routeDist; previewCost = t.routeCost; previewVisited = 0;
    char m[96];
    std::snprintf(m, sizeof(m), "%s dispatched to %s - route %.0f units", t.label().c_str(),
                  waste.bins[binId].label().c_str(), t.routeDist);
    waste.addLog(m);
    say(m);
    return true;
}

void Simulation::cmdDispatch() {
    if (!running()) { say("Start the simulation first"); return; }
    int t = pickIdleTruck();
    if (t < 0) { say("No idle truck available"); return; }
    int b = -1;
    if (selBin >= 0) {
        if (waste.bins[selBin].status != CS_WAITING) { say("That dustbin already has a truck"); return; }
        if (waste.bins[selBin].percent() < 1.0f) { say("That dustbin is empty"); return; }
        b = selBin;
    } else b = pickBestBin(1.0f);
    if (b < 0) { say("No dustbin needs collection"); return; }
    selTruck = t;
    selBin = b;
    dispatchPair(t, b);
}

void Simulation::cmdFindRoute() {
    if (selBin < 0) { say("Select a dustbin on the map first"); return; }
    int t = selTruck >= 0 ? selTruck : 0;
    if (selTruck < 0) selTruck = 0;
    float cost = 0, dist = 0; int vis = 0;
    // Dijkstra from the depot to the node next to the selected bin
    std::vector<int> r = planner.shortestPath(depotNode, waste.bins[selBin].nodeId, &cost, &dist, &vis);
    if (r.empty()) { say("No route found"); return; }
    previewRoute = r; previewDist = dist; previewCost = cost; previewVisited = vis;
    char m[112];
    std::snprintf(m, sizeof(m), "Shortest route for %s -> %s: %.0f units (cost %.0f, %d nodes explored)",
                  trucks[t].label().c_str(), waste.bins[selBin].label().c_str(), dist, cost, vis);
    waste.addLog(m);
    say(m);
}

void Simulation::cmdToggleTraffic() {
    traffic = !traffic;
    applyTraffic();
    say(traffic ? "Traffic ON - congested roads cost more, routes adapt" : "Traffic OFF");
    waste.addLog(traffic ? "Traffic enabled" : "Traffic disabled");
}
void Simulation::cmdToggleDayNight() {
    dayTarget = dayTarget > 0.5f ? 0.0f : 1.0f;
    say(dayTarget > 0.5f ? "Day mode" : "Night mode");
}
void Simulation::cmdCycleCategory() {
    if (selBin < 0) { say("Select a dustbin first"); return; }
    Dustbin& b = waste.bins[selBin];
    b.category = (b.category + 1) % W_COUNT;
    say(b.label() + " now collects " + wasteInfo(b.category).name + " waste");
}
void Simulation::cmdToggleAuto() {
    autoDispatch = !autoDispatch;
    say(autoDispatch ? "Auto dispatch ON (bins above 70%)" : "Auto dispatch OFF");
}
void Simulation::cmdCycleSpeed() {
    speed = (speed < 1.5f) ? 2.0f : (speed < 3.0f ? 4.0f : 1.0f);
    char m[32]; std::snprintf(m, sizeof(m), "Simulation speed x%.0f", speed);
    say(m);
}
