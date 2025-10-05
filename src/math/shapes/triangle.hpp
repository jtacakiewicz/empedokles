#ifndef EMP_TRIANGLE_HPP
#define EMP_TRIANGLE_HPP
#include "math/math_defs.hpp"
#include "math/math_func.hpp"
namespace emp {
struct Triangle {
    vec2f a;
    vec2f b;
    vec2f c;
    float calcMass(float thickness, float density) const
    {
        auto area = calcArea();
        return area * thickness * density;
    }
    float calcArea() const { return 0.5f * fabs(perp_dot(a - b, a - c)); }
    float calcMMOI(float mass) const
    {
        auto centroid = calcCentroid();
        auto A = a - centroid;
        auto B = b - centroid;
        auto C = c - centroid;

        auto aa = dot(A, A);
        auto bb = dot(B, B);
        auto cc = dot(C, C);
        auto ab = dot(A, B);
        auto bc = dot(B, C);
        auto ca = dot(C, A);
        return (aa + bb + cc + ab + bc + ca) * mass / 6.f;
    }
    vec2f calcCentroid() const { return (a + b + c) / 3.f; }
    operator std::vector<vec2f>() { return { a, b, c }; }
    Triangle(vec2f p1, vec2f p2, vec2f p3)
        : a(p1)
        , b(p2)
        , c(p3)
    {
    }
};
};
#endif
