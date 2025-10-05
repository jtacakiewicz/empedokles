#include "math_func.hpp"
#include <algorithm>
#include "math_defs.hpp"

namespace emp {
vec2f transformPoint(const TransformMatrix &mat, vec2f point)
{
    return mat * glm::vec4(point.x, point.y, 0.f, 1.f);
}
bool nearlyEqual(float a, float b, float dt)
{
    return abs(a - b) < dt;
}
bool nearlyEqual(vec2f a, vec2f b, float dt)
{
    return nearlyEqual(a.x, b.x, dt) && nearlyEqual(a.y, b.y, dt);
}
void sort_clockwise(std::vector<vec2f>::iterator begin, std::vector<vec2f>::iterator end)
{
    std::sort(begin, end, [](vec2f a, vec2f b) {
        auto anga = std::atan2(a.x, a.y);
        if(anga > EMP_PI) {
            anga -= 2.f * EMP_PI;
        } else if(anga <= -EMP_PI) {
            anga += 2.f * EMP_PI;
        }
        auto angb = std::atan2(b.x, b.y);
        if(angb > EMP_PI) {
            angb -= 2.f * EMP_PI;
        } else if(angb <= -EMP_PI) {
            angb += 2.f * EMP_PI;
        }

        return anga < angb;
    });
}
float angleAround(vec2f a, vec2f pivot, vec2f b)
{
    return angle(a - pivot, b - pivot);
}
vec2f sign(vec2f x)
{
    return { std::copysign(1.f, x.x), std::copysign(1.f, x.y) };
}
vec2f rotateVec(vec2f vec, float angle)
{
    return vec2f(cos(angle) * vec.x - sin(angle) * vec.y, sin(angle) * vec.x + cos(angle) * vec.y);
}

float qlen(vec2f v)
{
    return v.x * v.x + v.y * v.y;
}
float angle(vec2f a, vec2f b)
{
    return atan2(perp_dot(a, b), dot(a, b));
}
vec2f rotate(vec2f vec, float angle)
{
    return {
        cosf(angle) * vec.x - sinf(angle) * vec.y,
        sinf(angle) * vec.x + cosf(angle) * vec.y,
    };
}
float length(vec2f v)
{
    return sqrt(v.x * v.x + v.y * v.y);
}
float dot(vec2f a, vec2f b)
{
    return a.x * b.x + a.y * b.y;
}
vec2f normal(vec2f v)
{
    return v / length(v);
}
vec2f proj(vec2f a, vec2f plane_norm)
{
    return (dot(a, plane_norm) / dot(plane_norm, plane_norm)) * plane_norm;
}
float perp_dot(vec2f a, vec2f b)
{
    return a.x * b.y - b.x * a.y;
}

}  //  namespace emp
