#ifndef EMP_TYPES_HPP
#define EMP_TYPES_HPP
#include <math.h>
#include <cmath>
#include <iostream>
#include <set>
#include <vector>

#include "math/math_defs.hpp"

#include "math/math_func.hpp"
#include "shapes/AABB.hpp"
#include "shapes/circle.hpp"
#include "shapes/concave_polygon.hpp"
#include "shapes/convex_polygon.hpp"
#include "shapes/ray.hpp"
#include "shapes/triangle.hpp"

namespace emp {

vec2f operator*(vec2f a, vec2f b);
}  //  namespace emp
#endif
