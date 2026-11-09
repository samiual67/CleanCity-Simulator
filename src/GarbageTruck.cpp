#include "GarbageTruck.h"

static const float COLLECT_TIME = 1.8f;
static const float UNLOAD_TIME  = 1.2f;

void GarbageTruck::init(int truckId, Vec2 homePos, Color c) {
    id = truckId;
    home = homePos;
    color = c;
    mover = Mover();
    mover.pos = home;
    mover.angle = 0;
    mover.speed = 110.0f;
    state = T_IDLE;
    targetBin = -1;
    route.clear();
    routeDist = routeCost = 0;
    timer = progress = 0;
    wheelAngle = 0;
    collections = 0;
    totalCollected = 0;
    for (int i = 0; i < W_COUNT; i++) cargo[i] = 0;
}

std::string GarbageTruck::label() const {
    char b[16]; std::snprintf(b, sizeof(b), "TR-%d", id + 1); return b;
}

float GarbageTruck::cargoTotal() const {
    float s = 0; for (int i = 0; i < W_COUNT; i++) s += cargo[i]; return s;
}

std::string GarbageTruck::statusText() const {
    char b[64];
    switch (state) {
        case T_IDLE: return "Idle at depot";
        case T_TO_BIN: std::snprintf(b, sizeof(b), "En route to DB-%02d", targetBin + 1); return b;
        case T_COLLECTING: std::snprintf(b, sizeof(b), "Collecting DB-%02d (%d%%)", targetBin + 1, (int)(progress * 100)); return b;
        case T_RETURNING: return "Returning to depot";
        default: return "Unloading cargo";
    }
}

static std::vector<Vec2> routePoints(const RoutePlanner& rp, const std::vector<int>& r) {
    std::vector<Vec2> p;
    for (size_t i = 0; i < r.size(); i++) p.push_back(rp.nodes()[r[i]]);
    return p;
}

bool GarbageTruck::assign(int binId, const RoutePlanner& rp, WasteManager& wm, int depotNode) {
    if (state != T_IDLE || binId < 0 || binId >= (int)wm.bins.size()) return false;
    Dustbin& b = wm.bins[binId];
    float cost = 0, dist = 0;
    std::vector<int> r = rp.shortestPath(depotNode, b.nodeId, &cost, &dist);   // Dijkstra
    if (r.empty()) return false;
    route = r;
    routeDist = dist;
    routeCost = cost;
    mover.setPath(routePoints(rp, r));
    state = T_TO_BIN;
    targetBin = binId;
    b.status = CS_ASSIGNED;
    return true;
}

void GarbageTruck::update(float dt, const RoutePlanner& rp, WasteManager& wm, RecyclingCenter& rc, int depotNode) {
    bool moving = mover.active;
    switch (state) {
        case T_IDLE: break;

        case T_TO_BIN:
            if (mover.update(dt)) {
                state = T_COLLECTING;
                timer = 0; progress = 0;
                wm.bins[targetBin].status = CS_COLLECTING;
            }
            break;

        case T_COLLECTING: {
            timer += dt;
            progress = clampf(timer / COLLECT_TIME, 0, 1);
            if (timer >= COLLECT_TIME) {
                Dustbin& b = wm.bins[targetBin];
                float amount = b.empty();
                b.status = CS_WAITING;
                cargo[b.category] += amount;
                collections++;
                totalCollected += amount;
                wm.recordCollection(id, b, amount);
                char msg[96];
                std::snprintf(msg, sizeof(msg), "%s collected %.0f kg of %s from %s", label().c_str(),
                              amount, wasteInfo(b.category).name, b.label().c_str());
                wm.addLog(msg);
                // plan the way back with Dijkstra again
                std::vector<int> r = rp.shortestPath(b.nodeId, depotNode, &routeCost, &routeDist);
                route = r;
                std::vector<Vec2> pts = routePoints(rp, r);
                pts.push_back(home);
                mover.setPath(pts);
                state = T_RETURNING;
            }
            break;
        }

        case T_RETURNING:
            if (mover.update(dt)) { state = T_UNLOADING; timer = 0; progress = 0; }
            break;

        case T_UNLOADING:
            timer += dt;
            progress = clampf(timer / UNLOAD_TIME, 0, 1);
            if (timer >= UNLOAD_TIME) {
                char msg[96];
                std::snprintf(msg, sizeof(msg), "%s delivered %.0f kg to the Recycling Facility", label().c_str(), cargoTotal());
                wm.addLog(msg);
                rc.receive(cargo);
                for (int i = 0; i < W_COUNT; i++) cargo[i] = 0;
                state = T_IDLE;
                targetBin = -1;
                route.clear();
                mover.angle = 0;
            }
            break;
    }
    if (moving) wheelAngle += mover.speed * dt / 4.5f * 180.0f / PI_F;
}
