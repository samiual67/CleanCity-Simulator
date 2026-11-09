// RoutePlanner.h - road graph + Dijkstra's shortest path algorithm
#pragma once
#include "Common.h"

struct RoadEdge {
    int   a, b;
    float length;        // euclidean length (simulation units)
    float factor;        // congestion multiplier (1 = free road)
    float weight() const { return length * factor; }
};

class RoutePlanner {
public:
    int  addNode(Vec2 p);
    int  addEdge(int a, int b);
    void setEdgeFactor(int e, float f) { edges_[e].factor = f; }
    void clear() { nodes_.clear(); edges_.clear(); adj_.clear(); }

    // Dijkstra: returns node ids from s to t (empty if unreachable).
    // cost = sum of edge weights, dist = pure geometric length, visited = nodes settled.
    std::vector<int> shortestPath(int s, int t, float* cost = 0, float* dist = 0, int* visited = 0) const;

    int   nearestNode(const Vec2& p) const;
    std::vector<int> neighbors(int n) const;
    int   findEdge(int a, int b) const;

    const std::vector<Vec2>&     nodes() const { return nodes_; }
    const std::vector<RoadEdge>& edges() const { return edges_; }

private:
    std::vector<Vec2>             nodes_;
    std::vector<RoadEdge>         edges_;
    std::vector<std::vector<int>> adj_;      // adjacency list of edge indices
};
