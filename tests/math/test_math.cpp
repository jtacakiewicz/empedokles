#include <gtest/gtest.h>
#include <algorithm>
#include <random>
#include "math/math.hpp"
#include "math/math_func.hpp"

using namespace emp;
TEST(MathTest, BasicUtilFunctions)
{
    ASSERT_TRUE(nearlyEqual(0.f, 1e-10));
    ASSERT_EQ(length(vec2f(5, 0)), 5.f);
    ASSERT_EQ(normal(vec2f(1, 1)), normal(vec2f(100.f, 100.f)));
    ASSERT_EQ(length(normal(vec2f(5, 0))), 1.f);
    ASSERT_EQ(qlen(vec2f(5, 0)), 25.f);
    ASSERT_EQ(qlen(vec2f(4, 6)), powf(length(vec2f(4, 6)), 2.f));

    ASSERT_EQ(dot(vec2f(1, 0), vec2f(1, 0)), 1.f);
    ASSERT_EQ(dot(vec2f(2, 0), vec2f(2, 0)), 4.f);
    ASSERT_EQ(dot(vec2f(0, 2), vec2f(2, 0)), 0.f);

    ASSERT_EQ(proj(vec2f(2, 0), vec2f(0, 1)), vec2f(0.f, 0.f));
    ASSERT_EQ(proj(vec2f(2, 2), normal(vec2f(1, 1))), vec2f(2, 2));

    ASSERT_EQ(perp_dot(vec2f(1, 0), vec2f(1, 0)), 0.f);
    ASSERT_EQ(perp_dot(vec2f(0, -1), vec2f(1, 0)), 1.f);
    ASSERT_EQ(perp_dot(vec2f(0, 2), vec2f(2, 0)), -4.f);
    ASSERT_EQ(sign({ -21.f, 37.f }), vec2f(-1, 1));
    ASSERT_EQ(sign({ -69, -60 }), vec2f(-1, -1));
}
TEST(MathTest, Trigonometry)
{
    ASSERT_TRUE(nearlyEqual(angle(vec2f(1, 0), vec2f(0, 1)), M_PI / 2.f));
    ASSERT_EQ(angle(vec2f(1, 0), vec2f(1, 0)), 0.f);
    ASSERT_EQ(angle(vec2f(1, 0), vec2f(0, 1)), angleAround(vec2f(1, 0), vec2f(0, 0), vec2f(0, 1)));

    ASSERT_TRUE(nearlyEqual(angleAround(vec2f(2, 3), vec2f(2, 2), vec2f(3, 2)), -M_PI / 2.f));

    ASSERT_TRUE(nearlyEqual(rotate(vec2f(1, 0), M_PI / 2.f), vec2f(0, 1)));

    std::vector<vec2f> point_cloud;
    for(int i = 0; i < 10; i++) {
        float angle = i / 10.f * M_PI * 2.f;
        point_cloud.push_back({ cosf(angle), -sinf(angle) });
    }
    auto original = point_cloud;
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(point_cloud.begin(), point_cloud.end(), g);
    sort_clockwise(point_cloud.begin(), point_cloud.end());
    int beginning = 0;
    for(int i = 0; i < original.size(); i++) {
        if(original[i] == point_cloud.front()) {
            beginning = i;
            break;
        }
    }
    for(int i = 0; i < original.size(); i++) {
        ASSERT_EQ(original[(beginning + i) % original.size()], point_cloud[i]) << "for i = " << i;
    }
}
