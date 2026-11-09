// TransformLab.h - "Computer Graphics Concepts" screen: interactive 2D transformation demo
#pragma once
#include "Common.h"
#include "Transformation.h"
#include "UIComponents.h"

class TransformLab {
public:
    TransformLab();
    void reset();
    void draw(float time, float mouseX, float mouseY);
    bool click(float x, float y, float time);          // returns true when the click was used

    int   object;     // 0 truck, 1 tree, 2 building, 3 dustbin
    int   mode;       // 0 translate, 1 rotate, 2 scale, 3 reflect, 4 shear
    float tx, ty, angle, sx, sy, shx, shy;
    int   axis;

private:
    Mat3 base;        // cumulative matrix of committed transformations (math coordinates, y up)
    std::vector<Button> buttons;
    float animT;
    bool  animate;    // animated rotation demo

    Mat3 modeMatrix() const;
    void resetParams();
    void resetBase();
    void buildButtons();
    void drawObject(const Mat3& M, float alpha, float time);
    void drawBox(const Mat3& M, const Color& c, float lw);
    void drawMatrixPanel(float x, float y);
    void adjust(int row, int dir);
};
