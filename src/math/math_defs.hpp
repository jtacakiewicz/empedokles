#ifndef EMP_MATH_DEFS
#define EMP_MATH_DEFS
#include <math.h>
#include <cmath>
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/vector_float2.hpp"
#define EMP_PI  3.14159265358979323846264338327950288 /* pi */
#define fEMP_PI 3.141592653f                          /* pi */

typedef glm::mat4x4 TransformMatrix;
typedef glm::vec<4, float> vec4f;
typedef glm::vec<3, float> vec3f;
typedef glm::vec<2, float> vec2f;
typedef glm::vec<2, int> vec2i;

namespace emp {
}
#endif
