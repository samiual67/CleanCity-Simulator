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

static std::vector<Vec2> lanePath(const std::vector<Vec2>& in) {
    std::vector<Vec2> p;                                   // drop repeated points
    for (size_t i = 0; i < in.size(); i++)
        if (p.empty() || distf(p.back(), in[i]) > 0.001f) p.push_back(in[i]);
    std::vector<Vec2> o;
    if (p.size() < 2) return o;
    std::vector<Vec2> n;                                   // left-hand normal of every road segment
    for (size_t i = 0; i + 1 < p.size(); i++) {
        Vec2 d = p[i + 1] - p[i];
        float len = std::sqrt(d.x * d.x + d.y * d.y);
        n.push_back(Vec2(d.y / len, -d.x / len));
    }
    o.push_back(p.front() + n.front() * LANE_OFFSET);
    for (size_t i = 1; i + 1 < p.size(); i++) {            // mitre the lane at every junction
        Vec2 s = n[i - 1] + n[i];
        float den = 1.0f + n[i - 1].x * n[i].x + n[i - 1].y * n[i].y;
        o.push_back(den > 0.2f ? p[i] + s * (LANE_OFFSET / den) : p[i] + n[i] * LANE_OFFSET);
    }
    o.push_back(p.back() + n.back() * LANE_OFFSET);
    return o;
}

static std::vector<Vec2> departPath(const std::vector<Vec2>& road, const Vec2& home) {
    std::vector<Vec2> o = lanePath(road);
    if (o.empty()) return road;
    if (o.size() >= 2 && std::fabs(o[0].y - o[1].y) < 0.01f) o[0].x = home.x;   // start from this bay, no backtracking
    o.insert(o.begin(), Vec2(home.x, o.front().y));                              // leave the bay straight onto the road
    return o;
}

static std::vector<Vec2> returnPath(const std::vector<Vec2>& road, const Vec2& home) {
    std::vector<Vec2> o = lanePath(road);
    if (o.size() >= 2 && std::fabs(o[o.size() - 1].y - o[o.size() - 2].y) < 0.01f) o.back().x = home.x;   // stop in front of this bay
    if (!o.empty()) o.push_back(Vec2(home.x, o.back().y));                       // turn into the bay straight
    o.push_back(home);
    return o;
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
    mover.setPath(departPath(routePoints(rp, r), home));
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
                std::vector<Vec2> pts = returnPath(routePoints(rp, r), home);
                
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
            while (mover.angle > 180) mover.angle -= 360;                   // swing the truck straight in its bay
            while (mover.angle < -180) mover.angle += 360;
            mover.angle += (0 - mover.angle) * clampf(dt * 8.0f, 0.0f, 1.0f);
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
