#include "Gfx.h"
#include "GLInclude.h"

namespace Gfx {

static Mat3 cur;
static std::vector<Mat3> stack;
static int winW = WIN_W, winH = WIN_H;
static float gAlpha = 1.0f;
void setAlphaMul(float a) { gAlpha = a; }

void setWindowSize(int w, int h) { winW = w > 1 ? w : 1; winH = h > 1 ? h : 1; }

void setupProjection() {
    glViewport(0, 0, winW, winH);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, WIN_W, WIN_H, 0, -1, 1);       // y grows downwards, like UI coordinates
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POINT_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    cur = Mat3();
    stack.clear();
}

void resetMatrix() { cur = Mat3(); stack.clear(); }
void pushMatrix(const Mat3& m) { stack.push_back(cur); cur = cur * m; }
void popMatrix() { if (!stack.empty()) { cur = stack.back(); stack.pop_back(); } }
const Mat3& matrix() { return cur; }

void scissor(float x, float y, float w, float h) {
    float sx = (float)winW / WIN_W, sy = (float)winH / WIN_H;
    glEnable(GL_SCISSOR_TEST);
    glScissor((int)(x * sx), (int)(winH - (y + h) * sy), (int)(w * sx) + 1, (int)(h * sy) + 1);
}
void scissorOff() { glDisable(GL_SCISSOR_TEST); }

static inline void col(const Color& c) { glColor4f(c.r, c.g, c.b, c.a * gAlpha); }
static inline void vtx(float x, float y) { Vec2 p = cur.apply(Vec2(x, y)); glVertex2f(p.x, p.y); }

void rect(float x, float y, float w, float h, const Color& c) {
    col(c);
    glBegin(GL_QUADS);
    vtx(x, y); vtx(x + w, y); vtx(x + w, y + h); vtx(x, y + h);
    glEnd();
}
void rectGradient(float x, float y, float w, float h, const Color& top, const Color& bottom) {
    glBegin(GL_QUADS);
    col(top);    vtx(x, y); vtx(x + w, y);
    col(bottom); vtx(x + w, y + h); vtx(x, y + h);
    glEnd();
}
void rectGradientH(float x, float y, float w, float h, const Color& l, const Color& r) {
    glBegin(GL_QUADS);
    col(l); vtx(x, y); vtx(x, y + h);
    col(r); vtx(x + w, y + h); vtx(x + w, y);
    glEnd();
}
void rectOutline(float x, float y, float w, float h, const Color& c, float lw) {
    col(c); glLineWidth(lw);
    glBegin(GL_LINE_LOOP);
    vtx(x, y); vtx(x + w, y); vtx(x + w, y + h); vtx(x, y + h);
    glEnd();
}

static std::vector<Vec2> roundRectPts(float x, float y, float w, float h, float r) {
    r = std::min(r, std::min(w, h) / 2);
    std::vector<Vec2> p;
    const int seg = 6;
    float cx[4] = { x + w - r, x + w - r, x + r, x + r };
    float cy[4] = { y + r, y + h - r, y + h - r, y + r };
    float a0[4] = { -90, 0, 90, 180 };
    for (int k = 0; k < 4; k++)
        for (int i = 0; i <= seg; i++) {
            float a = (a0[k] + 90.0f * i / seg) * PI_F / 180.0f;
            p.push_back(Vec2(cx[k] + r * std::cos(a), cy[k] + r * std::sin(a)));
        }
    return p;
}
void roundRect(float x, float y, float w, float h, float r, const Color& c) {
    poly(roundRectPts(x, y, w, h, r), c);
}
void roundRectGradient(float x, float y, float w, float h, float r, const Color& top, const Color& bottom) {
    std::vector<Vec2> p = roundRectPts(x, y, w, h, r);
    glBegin(GL_POLYGON);
    for (size_t i = 0; i < p.size(); i++) {
        float t = clampf((p[i].y - y) / (h > 0 ? h : 1), 0, 1);
        col(mixColor(top, bottom, t));
        vtx(p[i].x, p[i].y);
    }
    glEnd();
}
void roundRectOutline(float x, float y, float w, float h, float r, const Color& c, float lw) {
    polyOutline(roundRectPts(x, y, w, h, r), c, lw, true);
}

void poly(const std::vector<Vec2>& pts, const Color& c) {
    col(c);
    glBegin(GL_POLYGON);
    for (size_t i = 0; i < pts.size(); i++) vtx(pts[i].x, pts[i].y);
    glEnd();
}
void polyOutline(const std::vector<Vec2>& pts, const Color& c, float lw, bool closed) {
    col(c); glLineWidth(lw);
    glBegin(closed ? GL_LINE_LOOP : GL_LINE_STRIP);
    for (size_t i = 0; i < pts.size(); i++) vtx(pts[i].x, pts[i].y);
    glEnd();
}
void triangle(Vec2 a, Vec2 b, Vec2 c, const Color& cc) {
    col(cc);
    glBegin(GL_TRIANGLES);
    vtx(a.x, a.y); vtx(b.x, b.y); vtx(c.x, c.y);
    glEnd();
}
void ellipse(float cx, float cy, float rx, float ry, const Color& c, int seg) {
    col(c);
    glBegin(GL_TRIANGLE_FAN);
    vtx(cx, cy);
    for (int i = 0; i <= seg; i++) {
        float a = 2 * PI_F * i / seg;
        vtx(cx + rx * std::cos(a), cy + ry * std::sin(a));
    }
    glEnd();
}
void circle(float cx, float cy, float r, const Color& c, int seg) { ellipse(cx, cy, r, r, c, seg); }
void circleOutline(float cx, float cy, float r, const Color& c, float lw, int seg) {
    col(c); glLineWidth(lw);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < seg; i++) {
        float a = 2 * PI_F * i / seg;
        vtx(cx + r * std::cos(a), cy + r * std::sin(a));
    }
    glEnd();
}
void arc(float cx, float cy, float r0, float r1, float a0, float a1, const Color& c) {
    int seg = std::max(4, (int)(std::fabs(a1 - a0) / 6.0f));
    col(c);
    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i <= seg; i++) {
        float a = (a0 + (a1 - a0) * i / seg) * PI_F / 180.0f;
        vtx(cx + r0 * std::cos(a), cy + r0 * std::sin(a));
        vtx(cx + r1 * std::cos(a), cy + r1 * std::sin(a));
    }
    glEnd();
}
void pie(float cx, float cy, float r, float a0, float a1, const Color& c) {
    int seg = std::max(3, (int)(std::fabs(a1 - a0) / 5.0f));
    col(c);
    glBegin(GL_TRIANGLE_FAN);
    vtx(cx, cy);
    for (int i = 0; i <= seg; i++) {
        float a = (a0 + (a1 - a0) * i / seg) * PI_F / 180.0f;
        vtx(cx + r * std::cos(a), cy + r * std::sin(a));
    }
    glEnd();
}
void line(float x1, float y1, float x2, float y2, const Color& c, float lw) {
    col(c); glLineWidth(lw);
    glBegin(GL_LINES);
    vtx(x1, y1); vtx(x2, y2);
    glEnd();
}
void point(float x, float y, float size, const Color& c) {
    col(c); glPointSize(size);
    glBegin(GL_POINTS);
    vtx(x, y);
    glEnd();
}
void dashedLine(float x1, float y1, float x2, float y2, const Color& c, float lw, float dash, float gap) {
    float dx = x2 - x1, dy = y2 - y1, len = std::sqrt(dx * dx + dy * dy);
    if (len < 0.001f) return;
    dx /= len; dy /= len;
    for (float d = 0; d < len; d += dash + gap) {
        float e = std::min(d + dash, len);
        line(x1 + dx * d, y1 + dy * d, x1 + dx * e, y1 + dy * e, c, lw);
    }
}

static void* fontPtr(FontSize f) {
    switch (f) {
        case FONT_S: return GLUT_BITMAP_HELVETICA_12;
        case FONT_M: return GLUT_BITMAP_HELVETICA_18;
        default:     return GLUT_BITMAP_TIMES_ROMAN_24;
    }
}
int textWidth(const std::string& s, FontSize f) {
    return glutBitmapLength(fontPtr(f), (const unsigned char*)s.c_str());
}
void text(float x, float y, const std::string& s, const Color& c, FontSize f) {
    Vec2 p = cur.apply(Vec2(x, y));
    col(c);
    glRasterPos2f(p.x, p.y);
    void* font = fontPtr(f);
    for (size_t i = 0; i < s.size(); i++) glutBitmapCharacter(font, (unsigned char)s[i]);
}
void textCentered(float cx, float y, const std::string& s, const Color& c, FontSize f) {
    text(cx - textWidth(s, f) / 2.0f, y, s, c, f);
}
void textRight(float rx, float y, const std::string& s, const Color& c, FontSize f) {
    text(rx - textWidth(s, f), y, s, c, f);
}

float strokeWidth(const std::string& s, float scale) {
    float w = 0;
    for (size_t i = 0; i < s.size(); i++) w += glutStrokeWidth(GLUT_STROKE_ROMAN, (unsigned char)s[i]);
    return w * scale;
}
void strokeText(float x, float y, const std::string& s, float scale, const Color& c, float lw) {
    Vec2 p = cur.apply(Vec2(x, y));
    col(c); glLineWidth(lw);
    glPushMatrix();
    glTranslatef(p.x, p.y, 0);
    glScalef(scale, -scale, 1);               // flip because our y axis points down
    for (size_t i = 0; i < s.size(); i++) glutStrokeCharacter(GLUT_STROKE_ROMAN, (unsigned char)s[i]);
    glPopMatrix();
}

} // namespace Gfx
