// Gfx.h - thin immediate-mode drawing layer on top of OpenGL.
// Every vertex is multiplied by our own 3x3 matrix stack (homogeneous coordinates),
// so all object placement in the project goes through Mat3.
#pragma once
#include "Common.h"
#include "Transformation.h"

enum FontSize { FONT_S = 0, FONT_M, FONT_L };

namespace Gfx {
    void setWindowSize(int w, int h);          // real window size in pixels
    void setAlphaMul(float a);                 // global alpha multiplier (used for ghost objects)
    void setupProjection();                    // logical 1280x720 ortho (y down)

    // matrix stack
    void resetMatrix();
    void pushMatrix(const Mat3& m);            // current = current * m
    void popMatrix();
    const Mat3& matrix();

    // clipping (logical coordinates)
    void scissor(float x, float y, float w, float h);
    void scissorOff();

    // primitives
    void rect(float x, float y, float w, float h, const Color& c);
    void rectGradient(float x, float y, float w, float h, const Color& top, const Color& bottom);
    void rectGradientH(float x, float y, float w, float h, const Color& left, const Color& right);
    void rectOutline(float x, float y, float w, float h, const Color& c, float lw = 1);
    void roundRect(float x, float y, float w, float h, float r, const Color& c);
    void roundRectGradient(float x, float y, float w, float h, float r, const Color& top, const Color& bottom);
    void roundRectOutline(float x, float y, float w, float h, float r, const Color& c, float lw = 1);
    void poly(const std::vector<Vec2>& pts, const Color& c);          // convex polygons
    void polyOutline(const std::vector<Vec2>& pts, const Color& c, float lw = 1, bool closed = true);
    void triangle(Vec2 a, Vec2 b, Vec2 c, const Color& col);
    void circle(float cx, float cy, float r, const Color& c, int seg = 32);
    void ellipse(float cx, float cy, float rx, float ry, const Color& c, int seg = 32);
    void circleOutline(float cx, float cy, float r, const Color& c, float lw = 1, int seg = 40);
    void arc(float cx, float cy, float r0, float r1, float a0, float a1, const Color& c); // degrees, ring band
    void pie(float cx, float cy, float r, float a0, float a1, const Color& c);
    void line(float x1, float y1, float x2, float y2, const Color& c, float lw = 1);
    void point(float x, float y, float size, const Color& c);
    void dashedLine(float x1, float y1, float x2, float y2, const Color& c, float lw, float dash, float gap);

    // text (bitmap fonts) - only the anchor position is transformed
    void text(float x, float y, const std::string& s, const Color& c, FontSize f = FONT_S);
    void textCentered(float cx, float y, const std::string& s, const Color& c, FontSize f = FONT_S);
    void textRight(float rx, float y, const std::string& s, const Color& c, FontSize f = FONT_S);
    int  textWidth(const std::string& s, FontSize f = FONT_S);
    // stroke text (scalable, used for titles). y is the baseline, height ~ 100*scale
    void strokeText(float x, float y, const std::string& s, float scale, const Color& c, float lw = 2);
    float strokeWidth(const std::string& s, float scale);
}
