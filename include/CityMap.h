// CityMap.h - rendering and picking of the virtual city map (pan / zoom camera)
#pragma once
#include "Common.h"
#include "Transformation.h"
#include "Simulation.h"

const float MAP_X = 0, MAP_Y = 56, MAP_W = 960, MAP_H = 664;

struct Camera {
    float panX, panY, zoom;
    Camera() { reset(); }
    void reset() { panX = panY = 0; zoom = 0.95f; }
    Mat3 matrix() const;                           // world -> screen (T * S * T), homogeneous
    Vec2 screenToWorld(const Vec2& s) const;
    Vec2 worldToScreen(const Vec2& w) const;
    void zoomBy(float factor, const Vec2& screenFocus);
    void clampPan();
};

class CityMapView {
public:
    Camera cam;
    void draw(const Simulation& sim);
    int  pickBin(const Simulation& sim, const Vec2& screen) const;     // -1 if none
    int  pickTruck(const Simulation& sim, const Vec2& screen) const;   // -1 if none
    static bool inside(float x, float y) { return x >= MAP_X && x < MAP_X + MAP_W && y >= MAP_Y && y < MAP_Y + MAP_H; }

private:
    void drawGround(const Simulation& sim);
    void drawBlock(const Simulation& sim, int bx, int by);
    void drawRoads(const Simulation& sim);
    void drawRoutes(const Simulation& sim);
    void drawLights(const Simulation& sim);
    void drawClouds(const Simulation& sim);
    void drawOverlayUI(const Simulation& sim);
    void drawSelectedPopup(const Simulation& sim);
};
