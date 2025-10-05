#include <gtest/gtest.h>
#include "core/coordinator.hpp"
#include "core/system.hpp"

using namespace emp;
class CoordinatorTest : public testing::Test {
protected:
    Coordinator coord;
};
TEST_F(CoordinatorTest, EntityCreation)
{
    auto entity1 = coord.createEntity();
    ASSERT_NE(coord.world(), entity1);
    auto entity2 = coord.createEntity();
    ASSERT_NE(entity1, entity2);
    ASSERT_TRUE(coord.isEntityAlive(entity1));
    ASSERT_TRUE(coord.isEntityAlive(entity2));
    ASSERT_TRUE(coord.isEntityAlive(coord.world()));
}
TEST_F(CoordinatorTest, EntityDestruction)
{
    auto entity1 = coord.createEntity();
    ASSERT_TRUE(coord.isEntityAlive(entity1));
    coord.destroyEntity(entity1);
    ASSERT_FALSE(coord.isEntityAlive(entity1));
}
struct TestComponent {
    float value;
};

TEST_F(CoordinatorTest, ComponentAddition)
{
    coord.registerComponent<TestComponent>();

    auto entity1 = coord.createEntity();
    static constexpr float comp_value = 42.f;
    coord.addComponent(entity1, TestComponent { comp_value });

    auto entity2 = coord.createEntity();

    auto comp1 = coord.getComponent<TestComponent>(entity1);
    auto comp2 = coord.getComponent<TestComponent>(entity2);

    ASSERT_NE(comp1, nullptr);
    ASSERT_EQ(comp1->value, comp_value);

    ASSERT_EQ(comp2, nullptr);

    coord.addComponent(entity2, TestComponent { comp_value * 2.f });

    auto comp1_new = coord.getComponent<TestComponent>(entity1);
    comp2 = coord.getComponent<TestComponent>(entity2);
    ASSERT_EQ(comp1, comp1_new);
    ASSERT_NE(comp2, nullptr);
    ASSERT_NE(comp1, comp2);
}
TEST_F(CoordinatorTest, ComponentDeletion)
{
    coord.registerComponent<TestComponent>();
    auto entity1 = coord.createEntity();
    auto entity2 = coord.createEntity();
    static constexpr float comp_value = 42.f;

    coord.addComponent(entity1, TestComponent { comp_value });
    coord.addComponent(entity2, TestComponent { comp_value * 2.f });
    ASSERT_TRUE(coord.hasComponent<TestComponent>(entity1));
    ASSERT_TRUE(coord.hasComponent<TestComponent>(entity2));

    coord.removeComponent<TestComponent>(entity1);
    ASSERT_FALSE(coord.hasComponent<TestComponent>(entity1));
    ASSERT_TRUE(coord.hasComponent<TestComponent>(entity2));

    coord.destroyEntity(entity2);
    ASSERT_FALSE(coord.hasComponent<TestComponent>(entity2));
}
struct TestSystem : public System<TestComponent> {
    void onEntityRemoved(Entity entity) override { messages.push({ false, entity }); }
    void onEntityAdded(Entity entity) override { messages.push({ true, entity }); }
    struct Msg {
        bool isAdded;
        Entity entity;
    };
    std::queue<Msg> messages;
};
TEST_F(CoordinatorTest, SystemUsage)
{
    coord.registerComponent<TestComponent>();
    coord.registerSystem<TestSystem>();
    auto system = coord.getSystem<TestSystem>();
    ASSERT_NE(system, nullptr);

    auto entity = coord.createEntity();
    coord.addComponent(entity, TestComponent());

    auto checkForAddition = [&]() {
        auto message = system->messages.front();
        ASSERT_EQ(message.entity, entity);
        ASSERT_TRUE(message.isAdded);
        system->messages.pop();
    };
    checkForAddition();

    coord.removeComponent<TestComponent>(entity);
    auto checkForDeletion = [&]() {
        auto message = system->messages.front();
        ASSERT_EQ(message.entity, entity);
        ASSERT_FALSE(message.isAdded);
        system->messages.pop();
    };
    checkForDeletion();

    coord.addComponent(entity, TestComponent());
    checkForAddition();
    //
    coord.destroyEntity(entity);
    checkForDeletion();
}
