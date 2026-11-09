// Objects.h - reusable drawing functions for city objects (local coordinates; callers set the matrix)
#pragma once
#include "Common.h"
#include "Gfx.h"

enum BuildingKind { BK_HOUSE = 0, BK_TOWER, BK_HOSPITAL, BK_SCHOOL, BK_MALL, BK_RECYCLE, BK_DEPOT };

namespace Objects {
    // truck: origin = centre, faces +x, about 44 x 20 units
    void drawTruck(const Color& body, float wheelAngle, float cargoFill, float night);
    // car: origin = centre, faces +x
    void drawCar(const Color& c, float night);
    // tree: origin = base of the trunk
    void drawTree(float sway, int variant);
    // building: origin = top-left corner of the front face, size w x h (2.5D oblique projection)
    void drawBuilding(int kind, float w, float h, float night, int variant, float time);
    // dustbin: origin = centre, about 10 x 14 units
    void drawBin(const Color& levelColor, const Color& catColor, float fill01, bool selected,
                 bool flashing, float lift, float time);
    void drawLightGlow(float x, float y, float night);
}
