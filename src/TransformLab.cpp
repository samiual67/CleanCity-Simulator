#include "TransformLab.h"
#include "Objects.h"
#include "WasteManager.h"

static const float OX = 384, OY = 392;                 // canvas origin on screen (logical coords)
static const float CX0 = 24, CY0 = 72, CW = 720, CH = 640;

// button ids
enum { B_OBJ = 0, B_MODE = 10, B_MINUS = 20, B_PLUS = 30, B_AXIS = 40, B_APPLY = 50, B_RESET = 51, B_ANIM = 52 };

static const char* OBJ_NAMES[4]  = { "Garbage Truck", "Tree", "Building", "Dustbin" };
static const char* MODE_NAMES[5] = { "Translation", "Rotation", "Scaling", "Reflection", "Shearing" };

TransformLab::TransformLab() : animT(0), animate(false) { object = 0; mode = 0; reset(); buildButtons(); }

void TransformLab::resetParams() { tx = 120; ty = -80; angle = 45; sx = 1.5f; sy = 1.5f; shx = 0.5f; shy = 0.0f; axis = 0; }

static float baseScale(int obj) {
    static const float s[4] = { 2.2f, 2.0f, 1.3f, 5.0f };
    return s[obj];
}

void TransformLab::resetBase() {
    base = makeTranslation(110, 70) * makeScaling(baseScale(object), baseScale(object));
}

void TransformLab::reset() { resetParams(); resetBase(); animate = false; }

Mat3 TransformLab::modeMatrix() const {
    switch (mode) {
        case 0: return makeTranslation(tx, ty);
        case 1: return makeRotation(animate ? animT : angle);
        case 2: return makeScaling(sx, sy);
        case 3: return makeReflection(axis);
        default: return makeShear(shx, shy);
    }
}

void TransformLab::buildButtons() {
    buttons.clear();
    float px = 760;
    for (int i = 0; i < 4; i++) buttons.push_back(Button(B_OBJ + i, px + i * 124, 104, 118, 32, OBJ_NAMES[i], C_BLUE));
    for (int i = 0; i < 5; i++) buttons.push_back(Button(B_MODE + i, px + i * 99, 176, 94, 32, MODE_NAMES[i], C_TEAL));
    buttons.push_back(Button(B_APPLY, px, 590, 160, 36, "Apply (commit)", C_GREEN));
    buttons.push_back(Button(B_RESET, px + 170, 590, 160, 36, "Reset object", C_ORANGE));
    buttons.push_back(Button(B_ANIM, px + 340, 590, 150, 36, "Animate rotation", C_BLUE));
}

void TransformLab::adjust(int row, int dir) {
    float d = (float)dir;
    switch (mode) {
        case 0: if (row == 0) tx += 20 * d; else ty += 20 * d; break;
        case 1: angle += 15 * d; break;
        case 2: if (row == 0) sx = clampf(sx + 0.25f * d, -3, 3); else sy = clampf(sy + 0.25f * d, -3, 3); break;
        case 4: if (row == 0) shx = clampf(shx + 0.25f * d, -2, 2); else shy = clampf(shy + 0.25f * d, -2, 2); break;
        default: break;
    }
}

bool TransformLab::click(float x, float y, float time) {
    (void)time;
    // clicking on the canvas in translation mode moves the object there
    if (x >= CX0 && x <= CX0 + CW && y >= CY0 && y <= CY0 + CH) {
        if (mode == 0) {
            Vec2 target(x - OX, -(y - OY));                      // screen -> math coordinates
            Vec2 c = base.apply(Vec2(0, 0));
            tx = std::floor(target.x - c.x); ty = std::floor(target.y - c.y);
        }
        return true;
    }
    for (size_t i = 0; i < buttons.size(); i++) {
        const Button& b = buttons[i];
        if (!b.contains(x, y)) continue;
        if (b.id >= B_OBJ && b.id < B_OBJ + 4) { object = b.id - B_OBJ; resetBase(); }
        else if (b.id >= B_MODE && b.id < B_MODE + 5) { mode = b.id - B_MODE; animate = false; }
        else if (b.id >= B_MINUS && b.id < B_MINUS + 2) adjust(b.id - B_MINUS, -1);
        else if (b.id >= B_PLUS && b.id < B_PLUS + 2) adjust(b.id - B_PLUS, +1);
        else if (b.id >= B_AXIS && b.id < B_AXIS + 4) axis = b.id - B_AXIS;
        else if (b.id == B_APPLY) { base = modeMatrix() * base; resetParams(); animate = false; }
        else if (b.id == B_RESET) reset();
        else if (b.id == B_ANIM) { mode = 1; animate = !animate; }
        return true;
    }
    return false;
}

// local bounding boxes (after centring) used to show the "before / after" outline
static void bbox(int obj, float& x0, float& y0, float& x1, float& y1) {
    switch (obj) {
        case 0: x0 = -24; y0 = -12; x1 = 24; y1 = 12; break;
        case 1: x0 = -11; y0 = -14; x1 = 11; y1 = 14; break;
        case 2: x0 = -26; y0 = -39; x1 = 26; y1 = 39; break;
        default: x0 = -7; y0 = -9; x1 = 7; y1 = 9; break;
    }
}

void TransformLab::drawBox(const Mat3& M, const Color& c, float lw) {
    float x0, y0, x1, y1;
    bbox(object, x0, y0, x1, y1);
    // bounding box points in local frame -> through M (math coordinates) -> screen
    Mat3 full = makeTranslation(OX, OY) * makeScaling(1, -1) * M;
    std::vector<Vec2> p;
    Vec2 q[4] = { Vec2(x0, -y0), Vec2(x1, -y0), Vec2(x1, -y1), Vec2(x0, -y1) };      // local y points down, math y up
    for (int i = 0; i < 4; i++) p.push_back(full.apply(q[i]));
    Gfx::polyOutline(p, c, lw, true);
}

void TransformLab::drawObject(const Mat3& M, float alpha, float time) {
    // Drawing matrix = Screen * M * F, F flips local (y down) shapes into math (y up) coordinates
    Gfx::pushMatrix(makeTranslation(OX, OY) * makeScaling(1, -1) * M * makeScaling(1, -1));
    Gfx::setAlphaMul(alpha);
    switch (object) {
        case 0: Objects::drawTruck(Color(0.16f, 0.72f, 0.42f), time * 90.0f, 0.5f, 0); break;
        case 1: Gfx::pushMatrix(makeTranslation(0, 12)); Objects::drawTree(std::sin(time * 1.5f), 0); Gfx::popMatrix(); break;
        case 2: Gfx::pushMatrix(makeTranslation(-26, -39)); Objects::drawBuilding(BK_TOWER, 52, 78, 0, 1, time); Gfx::popMatrix(); break;
        default: Objects::drawBin(C_ORANGE, wasteInfo(W_PLASTIC).color, 0.7f, false, false, 0, time); break;
    }
    Gfx::setAlphaMul(1.0f);
    Gfx::popMatrix();
}

void TransformLab::drawMatrixPanel(float x, float y) {
    Mat3 M = modeMatrix();
    Gfx::text(x, y, "Transformation matrix  M (3x3 homogeneous):", C_DIM);
    float mx = x + 6, my = y + 14;
    Gfx::line(mx, my, mx, my + 78, C_TEXT, 2); Gfx::line(mx, my, mx + 6, my, C_TEXT, 2); Gfx::line(mx, my + 78, mx + 6, my + 78, C_TEXT, 2);
    float rx = mx + 290;
    Gfx::line(rx, my, rx, my + 78, C_TEXT, 2); Gfx::line(rx, my, rx - 6, my, C_TEXT, 2); Gfx::line(rx, my + 78, rx - 6, my + 78, C_TEXT, 2);
    for (int r = 0; r < 3; r++)
        for (int c = 0; c < 3; c++) {
            char b[24]; std::snprintf(b, sizeof(b), "%.2f", std::fabs(M.m[r][c]) < 0.0005f ? 0.0f : M.m[r][c]);
            Gfx::textRight(mx + 100 + c * 92, my + 22 + r * 26, b, C_TEXT, FONT_M);
        }
}

void TransformLab::draw(float time, float mouseX, float mouseY) {
    if (animate) { animT += 0.6f * 1.0f; if (animT >= 360) animT -= 360; }
    // ---- canvas
    UI::panel(CX0, CY0, CW, CH);
    Gfx::scissor(CX0 + 2, CY0 + 2, CW - 4, CH - 4);
    for (float gx = OX - 360; gx <= OX + 360; gx += 40) Gfx::line(gx, CY0, gx, CY0 + CH, Color(1, 1, 1, gx == OX ? 0.0f : 0.06f), 1);
    for (float gy = OY - 320; gy <= OY + 320; gy += 40) Gfx::line(CX0, gy, CX0 + CW, gy, Color(1, 1, 1, gy == OY ? 0.0f : 0.06f), 1);
    Gfx::line(CX0 + 6, OY, CX0 + CW - 6, OY, Color(0.9f, 0.4f, 0.4f, 0.9f), 2);          // X axis
    Gfx::line(OX, CY0 + 6, OX, CY0 + CH - 6, Color(0.4f, 0.9f, 0.5f, 0.9f), 2);          // Y axis
    Gfx::text(CX0 + CW - 24, OY - 8, "X", Color(0.9f, 0.4f, 0.4f), FONT_M);
    Gfx::text(OX + 8, CY0 + 22, "Y", Color(0.4f, 0.9f, 0.5f), FONT_M);
    Gfx::text(OX + 6, OY + 16, "O (0,0)", C_DIM);
    if (mode == 3 && axis == 3) Gfx::dashedLine(OX - 320, OY + 320, OX + 320, OY - 320, Color(1, 0.8f, 0.2f, 0.8f), 1.5f, 8, 6);

    Mat3 before = base;
    Mat3 after = modeMatrix() * base;
    drawObject(before, 0.28f, time);                                   // ghost = original position
    drawBox(before, Color(1, 1, 1, 0.45f), 1.5f);
    drawObject(after, 1.0f, time);                                     // result
    drawBox(after, C_YELLOW, 2.0f);

    Vec2 c0 = before.apply(Vec2(0, 0)), c1 = after.apply(Vec2(0, 0));
    Vec2 s0(OX + c0.x, OY - c0.y), s1(OX + c1.x, OY - c1.y);
    Gfx::dashedLine(s0.x, s0.y, s1.x, s1.y, Color(0.3f, 1, 1, 0.7f), 1.5f, 6, 5);
    Gfx::circle(s0.x, s0.y, 3.5f, Color(1, 1, 1, 0.8f), 10);
    Gfx::circle(s1.x, s1.y, 3.5f, C_YELLOW, 10);
    char b[96];
    std::snprintf(b, sizeof(b), "before (%.0f, %.0f)", c0.x, c0.y);
    Gfx::text(s0.x + 8, s0.y + 16, b, Color(1, 1, 1, 0.75f));
    std::snprintf(b, sizeof(b), "after (%.0f, %.0f)", c1.x, c1.y);
    Gfx::text(s1.x + 8, s1.y - 10, b, C_YELLOW);
    Gfx::scissorOff();
    Gfx::text(CX0 + 14, CY0 + 22, "Transformation Canvas  (white = before, solid = after)", C_TEXT);
    if (mode == 0) Gfx::text(CX0 + 14, CY0 + CH - 12, "Tip: click anywhere on the canvas to translate the object there", C_DIM);

    // ---- control panel
    UI::panel(750, CY0, 506, CH, "Computer Graphics Concepts");
    Gfx::text(762, 170, "Transformation", C_DIM);

    for (size_t i = 0; i < buttons.size(); i++) {
        Button bt = buttons[i];
        bool sel = (bt.id >= B_OBJ && bt.id < B_OBJ + 4 && object == bt.id - B_OBJ) ||
                   (bt.id >= B_MODE && bt.id < B_MODE + 5 && mode == bt.id - B_MODE) ||
                   (bt.id == B_ANIM && animate);
        bt.toggled = sel;
        UI::button(bt, bt.contains(mouseX, mouseY));
    }

    // parameter rows
    float py = 238;
    Gfx::text(762, py - 12, std::string("Parameters - ") + MODE_NAMES[mode], C_DIM);
    int rows = 0; std::string names[2], vals[2];
    switch (mode) {
        case 0: rows = 2; names[0] = "tx (x shift)"; names[1] = "ty (y shift)";
                std::snprintf(b, sizeof(b), "%.0f", tx); vals[0] = b; std::snprintf(b, sizeof(b), "%.0f", ty); vals[1] = b; break;
        case 1: rows = 1; names[0] = "angle (deg, CCW)"; std::snprintf(b, sizeof(b), "%.0f", animate ? animT : angle); vals[0] = b; break;
        case 2: rows = 2; names[0] = "sx (scale x)"; names[1] = "sy (scale y)";
                std::snprintf(b, sizeof(b), "%.2f", sx); vals[0] = b; std::snprintf(b, sizeof(b), "%.2f", sy); vals[1] = b; break;
        case 4: rows = 2; names[0] = "shx (shear x)"; names[1] = "shy (shear y)";
                std::snprintf(b, sizeof(b), "%.2f", shx); vals[0] = b; std::snprintf(b, sizeof(b), "%.2f", shy); vals[1] = b; break;
        default: break;
    }
    // +/- buttons are drawn here (they depend on the mode) and handled through temporary entries
    std::vector<Button> tmp;
    for (int r = 0; r < rows; r++) {
        float ry = py + r * 44;
        Gfx::text(762, ry + 20, names[r], C_TEXT);
        Gfx::textCentered(1040, ry + 21, vals[r], C_YELLOW, FONT_M);
        tmp.push_back(Button(B_MINUS + r, 960, ry, 40, 30, "-", C_RED));
        tmp.push_back(Button(B_PLUS + r, 1080, ry, 40, 30, "+", C_GREEN));
    }
    if (mode == 3) {
        Gfx::text(762, py + 6, "Reflect about:", C_TEXT);
        for (int a = 0; a < 4; a++) {
            Button ab(B_AXIS + a, 762 + (a % 2) * 180, py + 18 + (a / 2) * 40, 170, 32, reflectionName(a), C_TEAL);
            ab.toggled = (axis == a);
            tmp.push_back(ab);
        }
    }
    for (size_t i = 0; i < tmp.size(); i++) UI::button(tmp[i], tmp[i].contains(mouseX, mouseY));
    // keep the dynamic buttons clickable: merge into the click list
    for (size_t i = 0; i < tmp.size(); i++) {
        bool found = false;
        for (size_t k = 0; k < buttons.size(); k++) if (buttons[k].id == tmp[i].id) { buttons[k] = tmp[i]; found = true; }
        if (!found) buttons.push_back(tmp[i]);
    }
    // remove stale dynamic buttons of other modes
    for (size_t k = buttons.size(); k-- > 0;) {
        int id = buttons[k].id;
        bool dynamic = (id >= B_MINUS && id < B_MINUS + 2) || (id >= B_PLUS && id < B_PLUS + 2) || (id >= B_AXIS && id < B_AXIS + 4);
        if (!dynamic) continue;
        bool keep = false;
        for (size_t i = 0; i < tmp.size(); i++) if (tmp[i].id == id) keep = true;
        if (!keep) buttons.erase(buttons.begin() + k);
    }

    drawMatrixPanel(762, 356);

    // formulas / explanation
    static const char* FORM[5] = {
        "x' = x + tx ,  y' = y + ty",
        "x' = x cos(a) - y sin(a) ,  y' = x sin(a) + y cos(a)",
        "x' = sx * x ,  y' = sy * y",
        "mirror about the selected axis (negates x and/or y, or swaps them)",
        "x' = x + shx * y ,  y' = y + shy * x"
    };
    Gfx::text(762, 478, "Formula:", C_DIM);
    Gfx::text(762, 498, FORM[mode], C_TEXT);
    Gfx::text(762, 520, "Every vertex is multiplied by [x y 1]^T with this matrix.", C_DIM);
    Gfx::text(762, 536, "Apply commits the result so transformations can be combined.", C_DIM);
    Gfx::text(762, 556, "All shapes (truck, tree, building, dustbin) are the same", C_DIM);
    Gfx::text(762, 572, "objects used in the city map.", C_DIM);
}
