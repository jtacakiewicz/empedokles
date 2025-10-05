
#include <gtest/gtest.h>
#include <algorithm>
#include <random>
#include "core/coordinator.hpp"
#include "math/geometry_func.hpp"
#include "math/math_func.hpp"
#include "math/shapes/circle.hpp"
#include "scene/transform.hpp"

using namespace emp;
TEST(TransformTest, Updates)
{
    Transform trans(vec2f(0, 0), 0.f, vec2f(1, 1));
    vec2f offset = vec2f(10, 10);
    trans.position += offset;
    trans.syncWithChange();
    vec2f point = { 0, 5 };
    vec2f tr_point = transformPoint(trans.global(), point);
    ASSERT_EQ(tr_point, point + offset);
}
TEST(TransformTest, Hiererchy)
{
    Coordinator ECS;
    ECS.registerComponent<Transform>();
    ECS.registerSystem<TransformSystem>();
    ECS.addComponent(ECS.world(), Transform(vec2f(0, 0), 0.f, { 1.f, 1.f }));

    auto parent = ECS.createEntity();
    auto child = ECS.createEntity();
    vec2f parent_pos = { 10, 10 };
    vec2f child_pos = { 5, 5 };
    {
        Transform transform(parent_pos, 0.f, { 2, 1 });
        ECS.addComponent(parent, transform);
    }
    {
        Transform transform(parent, child_pos);
        ECS.addComponent(child, transform);
    }
    ECS.getSystem<TransformSystem>()->update();
    auto &child_trans = *ECS.getComponent<Transform>(child);
    vec2f pos = transformPoint(child_trans.global(), vec2f(0));
    vec2f expected = parent_pos + vec2f(child_pos.x * 2.f, child_pos.y);
    ASSERT_EQ(expected, child_trans.getGlobalPosition());
    ASSERT_EQ(vec2f(2, 1), child_trans.getGlobalScale());
    ASSERT_EQ(0.f, child_trans.getGlobalRotation());
}
