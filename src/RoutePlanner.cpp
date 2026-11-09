#include "RoutePlanner.h"
#include <queue>
#include <limits>

int RoutePlanner::addNode(Vec2 p) {
    nodes_.push_back(p);
    adj_.push_back(std::vector<int>());
    return (int)nodes_.size() - 1;
}

int RoutePlanner::addEdge(int a, int b) {
    RoadEdge e;
    e.a = a; e.b = b; e.factor = 1.0f;
    e.length = distf(nodes_[a], nodes_[b]);
    edges_.push_back(e);
    int id = (int)edges_.size() - 1;
    adj_[a].push_back(id);
    adj_[b].push_back(id);
    return id;
}

std::vector<int> RoutePlanner::shortestPath(int s, int t, float* cost, float* dist, int* visited) const {
    const float INF = std::numeric_limits<float>::max();
    int n = (int)nodes_.size();
    std::vector<int> path;
    if (s < 0 || t < 0 || s >= n || t >= n) return path;

    std::vector<float> d(n, INF);
    std::vector<int> prev(n, -1);
    std::vector<char> done(n, 0);
    typedef std::pair<float, int> QItem;
    std::priority_queue<QItem, std::vector<QItem>, std::greater<QItem> > pq;
    d[s] = 0;
    pq.push(QItem(0.0f, s));
    int settled = 0;

    while (!pq.empty()) {
        QItem top = pq.top(); pq.pop();
        int u = top.second;
        if (done[u]) continue;
        done[u] = 1;
        settled++;
        if (u == t) break;
        for (size_t k = 0; k < adj_[u].size(); k++) {
            const RoadEdge& e = edges_[adj_[u][k]];
            int v = (e.a == u) ? e.b : e.a;
            float nd = d[u] + e.weight();
            if (nd < d[v]) {
                d[v] = nd;
                prev[v] = u;
                pq.push(QItem(nd, v));
            }
        }
    }
    if (visited) *visited = settled;
    if (d[t] == INF) return path;

    for (int v = t; v != -1; v = prev[v]) path.push_back(v);
    std::reverse(path.begin(), path.end());
    if (cost) *cost = d[t];
    if (dist) {
        float L = 0;
        for (size_t i = 1; i < path.size(); i++) L += distf(nodes_[path[i - 1]], nodes_[path[i]]);
        *dist = L;
    }
    return path;
}

int RoutePlanner::nearestNode(const Vec2& p) const {
    int best = -1; float bd = 1e30f;
    for (size_t i = 0; i < nodes_.size(); i++) {
        float d = distf(nodes_[i], p);
        if (d < bd) { bd = d; best = (int)i; }
    }
    return best;
}

std::vector<int> RoutePlanner::neighbors(int n) const {
    std::vector<int> r;
    for (size_t k = 0; k < adj_[n].size(); k++) {
        const RoadEdge& e = edges_[adj_[n][k]];
        r.push_back(e.a == n ? e.b : e.a);
    }
    return r;
}

int RoutePlanner::findEdge(int a, int b) const {
    for (size_t k = 0; k < adj_[a].size(); k++) {
        const RoadEdge& e = edges_[adj_[a][k]];
        if ((e.a == a && e.b == b) || (e.a == b && e.b == a)) return adj_[a][k];
    }
    return -1;
}
