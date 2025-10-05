#include <gtest/gtest.h>
#include <algorithm>
#include <random>
#include "math/geometry_func.hpp"
#include "math/shapes/circle.hpp"

using namespace emp;
TEST(GeometryTest, Overlaps)
{
    Circle circle { vec2f(5.f, 5.f), 5.f };
    ConvexPolygon poly(vec2f(-2.f, -2.f), 0.f, { vec2f(3.f, 3.f), vec2f(-3.f, 3.f), vec2f(0.f, -3.f) });
    AABB aabb1 = AABB::CreateFromCircle(circle);
    AABB aabb2 = AABB::CreateFromPolygon(poly);
    ASSERT_TRUE(isOverlappingAABBAABB(aabb1, aabb2));
    ASSERT_TRUE(isOverlappingPointAABB(vec2f(5.f, 5.f), aabb1));
    ASSERT_FALSE(isOverlappingPointAABB(vec2f(5.f, 5.f), aabb2));
    ASSERT_TRUE(isOverlappingPointCircle(vec2f(5.f + 2.f, 5.f + 3.f), circle));
    ASSERT_FALSE(isOverlappingPointCircle(vec2f(0, 0), circle));
    ASSERT_TRUE(isOverlappingPointPoly(vec2f(-2, -2), poly.getVertecies()));
    ASSERT_TRUE(isOverlappingPointPoly(vec2f(-1.5, -3), poly.getVertecies()));
}
TEST(GeometryTest, Intersection) { }
