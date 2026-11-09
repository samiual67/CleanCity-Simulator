#include "Transformation.h"

Mat3::Mat3() {
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++) m[i][j] = (i == j) ? 1.0f : 0.0f;
}

Mat3 Mat3::operator*(const Mat3& o) const {
    Mat3 r;
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++) {
            float s = 0;
            for (int k = 0; k < 3; k++) s += m[i][k] * o.m[k][j];
            r.m[i][j] = s;
        }
    return r;
}

Vec2 Mat3::apply(const Vec2& p) const {
    return Vec2(m[0][0] * p.x + m[0][1] * p.y + m[0][2],
                m[1][0] * p.x + m[1][1] * p.y + m[1][2]);
}

Mat3 Mat3::inverse() const {
    float a = m[0][0], b = m[0][1], c = m[1][0], d = m[1][1];
    float det = a * d - b * c;
    Mat3 r;
    if (std::fabs(det) < 1e-9f) return r;
    float ia = d / det, ib = -b / det, ic = -c / det, id = a / det;
    r.m[0][0] = ia; r.m[0][1] = ib; r.m[1][0] = ic; r.m[1][1] = id;
    r.m[0][2] = -(ia * m[0][2] + ib * m[1][2]);
    r.m[1][2] = -(ic * m[0][2] + id * m[1][2]);
    return r;
}

Mat3 makeTranslation(float tx, float ty) {
    Mat3 r; r.m[0][2] = tx; r.m[1][2] = ty; return r;
}
Mat3 makeRotation(float degrees) {
    float a = degrees * PI_F / 180.0f, c = std::cos(a), s = std::sin(a);
    Mat3 r; r.m[0][0] = c; r.m[0][1] = -s; r.m[1][0] = s; r.m[1][1] = c; return r;
}
Mat3 makeScaling(float sx, float sy) {
    Mat3 r; r.m[0][0] = sx; r.m[1][1] = sy; return r;
}
Mat3 makeReflection(int axis) {
    Mat3 r;
    switch (axis) {
        case 0: r.m[1][1] = -1; break;                                   // about X axis
        case 1: r.m[0][0] = -1; break;                                   // about Y axis
        case 2: r.m[0][0] = -1; r.m[1][1] = -1; break;                   // about origin
        default: r.m[0][0] = 0; r.m[1][1] = 0; r.m[0][1] = 1; r.m[1][0] = 1; break; // y = x
    }
    return r;
}
Mat3 makeShear(float shx, float shy) {
    Mat3 r; r.m[0][1] = shx; r.m[1][0] = shy; return r;
}
const char* reflectionName(int axis) {
    static const char* n[] = { "X-axis", "Y-axis", "Origin", "Line y = x" };
    return n[axis & 3];
}
