#include "CityLayout.h"

namespace City {

static const BlockType LAYOUT[3][4] = {
    { B_RES,      B_SCHOOL, B_PARK, B_COM },
    { B_HOSPITAL, B_LAKE,   B_SHOP, B_RES },
    { B_COM,      B_RES,    B_RECYCLE, B_DEPOT }
};

BlockType blockType(int bx, int by) { return LAYOUT[by][bx]; }

const char* blockName(BlockType t) {
    switch (t) {
        case B_RES: return "Residential";
        case B_COM: return "Commercial";
        case B_HOSPITAL: return "Hospital";
        case B_SCHOOL: return "School";
        case B_PARK: return "Public Park";
        case B_SHOP: return "Shopping Area";
        case B_LAKE: return "Lake";
        case B_RECYCLE: return "Recycling Facility";
        default: return "Collection Center";
    }
}

Vec2 nodePos(int i, int j) { return Vec2(X0 + i * DX, Y0 + j * DY); }

void buildGraph(RoutePlanner& rp, int& depotNode) {
    rp.clear();
    for (int j = 0; j < ROWS; j++)
        for (int i = 0; i < COLS; i++) rp.addNode(nodePos(i, j));        // node id = j*COLS + i
    // depot sits in the middle of the bottom road, between node 18 and node 19
    Vec2 a = nodePos(3, 3), b = nodePos(4, 3);
    depotNode = rp.addNode(Vec2((a.x + b.x) / 2, a.y));

    for (int j = 0; j < ROWS; j++)
        for (int i = 0; i < COLS; i++) {
            int n = j * COLS + i;
            if (i < COLS - 1) {
                if (j == ROWS - 1 && i == COLS - 2) {          // split the road around the depot
                    rp.addEdge(n, depotNode);
                    rp.addEdge(depotNode, n + 1);
                } else rp.addEdge(n, n + 1);
            }
            if (j < ROWS - 1) rp.addEdge(n, n + COLS);
        }
}

bool isCongested(int e) { return (e % 4) == 1; }

} // namespace City
