#ifndef EMP_RAY_HPP
#define EMP_RAY_HPP
#include "math/math_defs.hpp"
#include "math/math_func.hpp"
namespace emp {
struct Ray {
    vec2f pos;
    vec2f dir;

    float length() const { return emp::length(dir); }
    Ray() { }
    static Ray CreatePoints(vec2f a, vec2f b);
    static Ray CreatePositionDirection(vec2f p, vec2f d);
};

}
#endif
