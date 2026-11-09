// Application.h - window-level application: screens, buttons, input handling
#pragma once
#include "Common.h"
#include "Simulation.h"
#include "CityMap.h"
#include "Dashboard.h"
#include "TransformLab.h"
#include "UIComponents.h"

enum Screen { SCR_SPLASH = 0, SCR_DASH, SCR_MAP, SCR_RECYCLE, SCR_STATS, SCR_LAB };

class Application {
public:
    bool wantsExit;

    Application();
    void init();
    void setWindowSize(int w, int h);
    void update(float dt);
    void render();

    void onMouse(int button, int state, int x, int y);
    void onMotion(int x, int y);
    void onPassiveMotion(int x, int y);
    void onWheel(int dir, int x, int y);
    void onKey(unsigned char key);
    void onSpecial(int key);

private:
    Simulation   sim;
    CityMapView  mapView;
    Dashboard    dashboard;
    TransformLab lab;
    Screen screen;
    bool showAbout, showReport;
    float mx, my;                       // mouse in logical coordinates
    int   winW, winH;
    bool  dragging, dragMoved;
    float pressX, pressY, lastX, lastY;

    std::vector<Button> splashButtons, navButtons, controlButtons, reportButton;
    Button categoryButton;

    void buildButtons();
    void handleButton(int id);
    Vec2 toLogical(int x, int y) const;
    void mapClick(float x, float y);

    void drawSplash();
    void drawNav();
    void drawMapScreen();
    void drawControlPanel();
    void drawAbout();
};
