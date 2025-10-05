#include "coordinator.hpp"
#include "debug/debug.hpp"
namespace emp {
const Entity Coordinator::world()
{
    return 0U;
}
Coordinator::Coordinator()
{
    //  Create pointers to each manager
    auto world_entity = createEntity();
    assert(world_entity == world());
}

//  Entity methods
Entity Coordinator::createEntity()
{
    auto result = m_entity_manager.createEntity();
    EMP_DEBUGCALL(EMP_LOG(DEBUG2) << "entity created: " << result;)
    return result;
}
bool Coordinator::isEntityAlive(Entity entity) const
{
    return m_entity_manager.isEntityAlive(entity);
}

void Coordinator::destroyEntity(Entity entity)
{
    EMP_DEBUGCALL(EMP_LOG(DEBUG2) << "entity destroyed: " << entity;)
    m_system_manager.EntityDestroyed(entity);
    m_component_manager.EntityDestroyed(entity);
    m_entity_manager.destroyEntity(entity);
}
};
