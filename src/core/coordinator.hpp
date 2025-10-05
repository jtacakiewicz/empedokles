#ifndef EMP_COORDINATOR_HPP
#define EMP_COORDINATOR_HPP
#include "component_manager.hpp"
#include "debug/debug.hpp"
#include "entity_manager.hpp"
#include "system_manager.hpp"
namespace emp {
class Coordinator {
public:
    Coordinator();
    static const Entity world();

    //  Entity methods
    Entity createEntity();
    bool isEntityAlive(Entity entity) const;
    void destroyEntity(Entity entity);

    template <typename T> inline void registerComponent()
    {
        EMP_DEBUGCALL(EMP_LOG(DEBUG2) << "registered component: " << typeid(T).name();)
        m_component_manager.registerComponent<T>();
    }

    template <typename T> inline void addComponent(Entity entity, T component)
    {
        m_component_manager.addComponent<T>(entity, component);

        auto signature = m_entity_manager.getSignature(entity);
        signature.set(m_component_manager.getComponentType<T>(), true);
        m_entity_manager.setSignature(entity, signature);

        m_system_manager.EntitySignatureChanged(entity, signature);
    }

    template <typename T> inline void removeComponentIfExists(Entity entity)
    {
        if(!hasComponent<T>(entity)) {
            return;
        }
        removeComponent<T>(entity);
    }
    template <typename T> inline void removeComponent(Entity entity)
    {
        m_component_manager.removeComponent<T>(entity);

        auto signature = m_entity_manager.getSignature(entity);
        signature.set(m_component_manager.getComponentType<T>(), false);
        m_entity_manager.setSignature(entity, signature);

        m_system_manager.EntitySignatureChanged(entity, signature);
    }

    template <typename T> inline const T *getComponent(Entity entity) const
    {
        if(!hasComponent<T>(entity)) {
            return nullptr;
        }
        return &m_component_manager.getComponent<T>(entity);
    }

    template <typename T> inline T *getComponent(Entity entity)
    {
        if(!hasComponent<T>(entity)) {
            return nullptr;
        }
        return &m_component_manager.getComponent<T>(entity);
    }

    template <typename T> inline ComponentType getComponentType() { return m_component_manager.getComponentType<T>(); }
    template <typename T> inline bool hasComponent(Entity entity) const { return m_component_manager.hasComponent<T>(entity); }

    //  System methods
    template <typename SystemType, class... InitalizerValues> SystemType &registerSystem(InitalizerValues... inits)
    {
        EMP_DEBUGCALL(EMP_LOG(DEBUG2) << "registered system: " << typeid(SystemType).name(););
        auto &system = m_system_manager.registerSystem<SystemType>(inits...);
        system.setECS(this);

        auto system_signature = m_getSignatureSystemOf<SystemType>(system);
        m_setSystemSignature<SystemType>(system_signature);
        return system;
    }
    template <typename SystemType> inline SystemType *getSystem() { return m_system_manager.getSystem<SystemType>(); }

private:
    template <class OgSystemType, class... ComponentType> Signature m_getSignatureSystemOf(SystemOf<ComponentType...> &system)
    {
        static_assert(std::derived_from<OgSystemType, SystemOf<ComponentType...>> == true &&
                      "registered system must be of type SystemOf<Components>");

        Signature system_signature;
        (system_signature.set(m_component_manager.getComponentType<ComponentType>()), ...);
        return system_signature;
    }

    template <typename T> void m_setSystemSignature(Signature signature) { m_system_manager.setSignature<T>(signature); }

    ComponentManager m_component_manager;
    EntityManager m_entity_manager;
    SystemManager m_system_manager;
};
};  //  namespace emp
#endif
