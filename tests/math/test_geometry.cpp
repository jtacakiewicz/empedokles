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
TEST(GeometryTest, Intersection)
{
    //  convex polygons only
    const std::vector<vec2f> square1 { vec2f(0.f, 0.f), vec2f(2.f, 0.f), vec2f(2.f, 2.f), vec2f(0.f, 2.f) };
    const std::vector<vec2f> square2 { vec2f(1.f, 1.f), vec2f(3.f, 1.f), vec2f(3.f, 3.f), vec2f(1.f, 3.f) };
    const std::vector<vec2f> square_far { vec2f(10.f, 10.f), vec2f(12.f, 10.f), vec2f(12.f, 12.f), vec2f(10.f, 12.f) };
    const std::vector<vec2f> square_touching { vec2f(2.f, 0.f), vec2f(4.f, 0.f), vec2f(4.f, 2.f), vec2f(2.f, 2.f) };
    const std::vector<vec2f> square_inside { vec2f(0.5f, 0.5f), vec2f(1.5f, 0.5f), vec2f(1.5f, 1.5f), vec2f(0.5f, 1.5f) };
    const std::vector<vec2f> triangle { vec2f(0.f, 0.f), vec2f(4.f, 0.f), vec2f(4.f, 2.f) };

    //  no overlap at all
    auto far_apart = intersectPolygonPolygon(square1, square_far);
    ASSERT_FALSE(far_apart.detected);
    ASSERT_FALSE(intersectPolygonPolygon(square_far, square1).detected);

    //  edges are only touching -> no real intersection
    ASSERT_FALSE(intersectPolygonPolygon(square1, square_touching).detected);

    //  partially overlapping squares
    auto partial = intersectPolygonPolygon(square1, square2);
    ASSERT_TRUE(partial.detected);
    EXPECT_FLOAT_EQ(partial.overlap, 1.f);
    EXPECT_FLOAT_EQ(std::abs(partial.contact_normal.x), 0.f);
    EXPECT_FLOAT_EQ(std::abs(partial.contact_normal.y), 1.f);
    //  both contact points lie on the shared edge of the first square
    EXPECT_NEAR(partial.cp1.y, 0.f, 0.0001f);
    EXPECT_GE(partial.cp1.x, 0.f);
    EXPECT_LE(partial.cp1.x, 2.f);
    EXPECT_NEAR(partial.cp2.y, 0.f, 0.0001f);
    EXPECT_GE(partial.cp2.x, 0.f);
    EXPECT_LE(partial.cp2.x, 2.f);

    //  intersection must be detected regardless of the argument order
    auto partial_reversed = intersectPolygonPolygon(square2, square1);
    ASSERT_TRUE(partial_reversed.detected);
    EXPECT_FLOAT_EQ(partial_reversed.overlap, partial.overlap);

    //  identical polygons
    auto identical = intersectPolygonPolygon(square1, square1);
    ASSERT_TRUE(identical.detected);
    EXPECT_FLOAT_EQ(identical.overlap, 2.f);

    //  one polygon fully contained in the other
    auto contained = intersectPolygonPolygon(square1, square_inside);
    ASSERT_TRUE(contained.detected);
    EXPECT_FLOAT_EQ(contained.overlap, 1.f);

    //  non axis aligned convex polygons
    auto triangle_square = intersectPolygonPolygon(triangle, square2);
    ASSERT_TRUE(triangle_square.detected);
    EXPECT_GT(triangle_square.overlap, 0.f);
    EXPECT_LE(triangle_square.overlap, 1.f);
}
