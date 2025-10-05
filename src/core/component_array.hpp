#ifndef EMP_COMPONENT_ARRAY_HPP
#define EMP_COMPONENT_ARRAY_HPP
#include <array>
#include <unordered_map>
#include "core/entity.hpp"
namespace emp {
template <typename T, size_t N> constexpr auto make_array(T value) -> std::array<T, N>
{
    std::array<T, N> a {};
    for(auto &x : a) {
        x = value;
    }
    return a;
}
class IComponentArray {
public:
    virtual ~IComponentArray() = default;
    virtual void EntityDestroyed(Entity entity) = 0;
};
template <typename T> class ComponentArray : public IComponentArray {
public:
    void InsertData(Entity entity, T component)
    {
        assert(m_entity_to_index_map[entity] == INVALID_INDEX && "Component added to same entity more than once.");

        //  Put new entry at end and update the maps
        size_t new_index = m_size;
        m_entity_to_index_map[entity] = new_index;
        m_index_to_entity_map[new_index] = entity;
        m_component_array[new_index] = component;
        ++m_size;
    }

    void RemoveData(Entity entity)
    {
        assert(m_entity_to_index_map[entity] != INVALID_INDEX && "Removing non-existent component.");

        //  Copy element at end into deleted element's place to maintain density
        size_t index_of_removed_entity = m_entity_to_index_map[entity];
        size_t index_of_last_element = m_size - 1;
        m_component_array[index_of_removed_entity] = m_component_array[index_of_last_element];

        //  Update map to point to moved spot
        Entity entity_of_last_element = m_index_to_entity_map[index_of_last_element];
        m_entity_to_index_map[entity_of_last_element] = index_of_removed_entity;
        m_index_to_entity_map[index_of_removed_entity] = entity_of_last_element;

        m_entity_to_index_map[entity] = INVALID_INDEX;
        m_index_to_entity_map[index_of_last_element] = INVALID_ENTITY;

        --m_size;
    }

    bool hasData(Entity entity) const
    {
        if(entity >= m_entity_to_index_map.size()) {
            return false;
        }
        return m_entity_to_index_map[entity] != INVALID_INDEX;
    }
    T &GetData(Entity entity)
    {
        assert(m_entity_to_index_map[entity] != INVALID_INDEX && "Retrieving non-existent component.");
        //  Return a reference to the entity's component
        return m_component_array[m_entity_to_index_map[entity]];
    }
    const T &GetData(Entity entity) const
    {
        assert(m_entity_to_index_map[entity] != INVALID_INDEX && "Retrieving non-existent component.");
        //  Return a reference to the entity's component
        return m_component_array[m_entity_to_index_map[entity]];
    }

    void EntityDestroyed(Entity entity) override
    {
        if(m_entity_to_index_map[entity] != INVALID_INDEX) {
            //  Remove the entity's component if it existed
            RemoveData(entity);
        }
    }

private:
    static constexpr size_t INVALID_INDEX = -1U;
    static constexpr Entity INVALID_ENTITY = -1U;
    std::array<T, MAX_ENTITIES> m_component_array;
    std::array<size_t, MAX_ENTITIES> m_entity_to_index_map = make_array<size_t, MAX_ENTITIES>(INVALID_INDEX);
    std::array<Entity, MAX_ENTITIES> m_index_to_entity_map = make_array<Entity, MAX_ENTITIES>(INVALID_ENTITY);
    size_t m_size;
};

};  //  namespace emp
#endif
