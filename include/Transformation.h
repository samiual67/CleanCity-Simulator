// Transformation.h - 3x3 homogeneous coordinate matrices for 2D transformations
#pragma once
#include "Common.h"

struct Mat3 {
    float m[3][3];
    Mat3();                                  // identity
    Mat3 operator*(const Mat3& o) const;     // matrix product (this * o)
    Vec2 apply(const Vec2& p) const;         // transform a point (x, y, 1)
    Mat3 inverse() const;                    // inverse of an affine matrix
};

// The five required 2D transformations (all return 3x3 matrices)
Mat3 makeTranslation(float tx, float ty);
Mat3 makeRotation(float degrees);                 // counter-clockwise, about the origin
Mat3 makeScaling(float sx, float sy);             // about the origin
Mat3 makeReflection(int axis);                    // 0: X-axis, 1: Y-axis, 2: origin, 3: line y = x
Mat3 makeShear(float shx, float shy);             // x' = x + shx*y ; y' = y + shy*x

const char* reflectionName(int axis);
