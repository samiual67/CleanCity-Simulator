// CityLayout.h - geometry of the virtual city (road grid, blocks, zones)
#pragma once
#include "Common.h"
#include "RoutePlanner.h"

namespace City {
    const int   COLS = 5, ROWS = 4;               // intersections
    const float X0 = 100, Y0 = 80, DX = 200, DY = 147;
    const float ROAD_W = 26;
    const float WORLD_W = 1000, WORLD_H = 600;

    enum BlockType { B_RES, B_COM, B_HOSPITAL, B_SCHOOL, B_PARK, B_SHOP, B_LAKE, B_RECYCLE, B_DEPOT };

    BlockType blockType(int bx, int by);          // bx 0..3, by 0..2
    const char* blockName(BlockType t);
    Vec2 nodePos(int i, int j);
    void buildGraph(RoutePlanner& rp, int& depotNode);   // grid roads + depot node
    bool isCongested(int edgeIndex);                      // roads that jam when traffic is ON
}
