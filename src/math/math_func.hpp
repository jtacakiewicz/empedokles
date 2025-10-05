#ifndef EMP_MATH_FUNC_HPP
#define EMP_MATH_FUNC_HPP
#include <vector>
#include "math_defs.hpp"
namespace emp {

vec2f transformPoint(const TransformMatrix &mat, vec2f point);
//  vec2f transformPoint(const glm::mat4x4& transform, vec2f point);
#define VERY_SMALL_AMOUNT 1e-5
bool nearlyEqual(float a, float b, float dt = VERY_SMALL_AMOUNT);
//  returns true if a and b are nearly equal
bool nearlyEqual(vec2f a, vec2f b, float dt = VERY_SMALL_AMOUNT);
void sort_clockwise(std::vector<vec2f>::iterator begin, std::vector<vec2f>::iterator end);
vec2f rotate(vec2f vec, float angle);
float length(vec2f v);
float qlen(vec2f);
vec2f normal(vec2f v);
float dot(vec2f a, vec2f b);
vec2f proj(vec2f a, vec2f plane_norm);
float perp_dot(vec2f a, vec2f b);

float angleAround(vec2f a, vec2f pivot, vec2f b);
//  around origin point
float angle(vec2f, vec2f);
vec2f sign(vec2f);
vec2f rotateVec(vec2f vec, float angle);

}  //  namespace emp
#endif  //  EMP_MATH_FUNC_HPP
