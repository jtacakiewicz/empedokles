#ifndef EMP_ENTITY_MANAGER_HPP
#define EMP_ENTITY_MANAGER_HPP
#include <array>
#include <cassert>
#include <cstdint>
#include <queue>
#include "core/component.hpp"
#include "entity.hpp"
namespace emp {
class EntityManager {
public:
    EntityManager();

    Entity createEntity();
    bool isEntityAlive(Entity entity) const;
    void destroyEntity(Entity entity);
    void setSignature(Entity entity, Signature signature);
    Signature getSignature(Entity entity) const;

private:
    //  Queue of unused entity IDs
    std::queue<Entity> m_available_entities {};

    //  Array of signatures where the index corresponds to the entity ID
    std::array<Signature, MAX_ENTITIES> m_signatures;

    //  Total living entities - used to keep limits on how many exist
    uint32_t m_living_entity_count {};
};
};  //  namespace emp
#endif
