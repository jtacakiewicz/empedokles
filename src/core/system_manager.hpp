#ifndef EMP_SYSTEM_MANAGER_HPP
#define EMP_SYSTEM_MANAGER_HPP
#include <set>
#include <unordered_map>
#include "core/component.hpp"
#include "core/entity.hpp"
#include "core/system_base.hpp"
namespace emp {
class SystemManager {
public:
    template <typename T, class... InitializerValues> T &registerSystem(InitializerValues... inits)
    {
        std::size_t type_code = typeid(T).hash_code();
        assert(m_systems.find(type_code) == m_systems.end() && "Registering system more than once.");

        auto system = std::make_unique<T>(inits...);
        auto &ref = *system.get();
        m_systems.insert({ type_code, std::move(system) });
        return ref;
    }
    template <typename T> T *getSystem()
    {
        std::size_t type_code = typeid(T).hash_code();
        return m_systems.contains(type_code) ? dynamic_cast<T *>(m_systems.at(type_code).get()) : nullptr;
    }

    template <typename T> void setSignature(Signature signature)
    {
        std::size_t type_code = typeid(T).hash_code();
        assert(m_systems.find(type_code) != m_systems.end() && "System used before registered.");

        m_signatures.insert({ type_code, signature });
    }

    void EntityDestroyed(Entity entity)
    {
        for(const auto &pair : m_systems) {
            const auto &system = pair.second;

            system->onEntityRemoved(entity);
            system->entities.erase(entity);
        }
    }

    void EntitySignatureChanged(Entity entity, Signature entitySignature)
    {
        for(const auto &pair : m_systems) {
            const auto &type = pair.first;
            const auto &system = pair.second;
            const auto &systemSignature = m_signatures[type];
            const bool contained = system->entities.contains(entity);

            if((entitySignature & systemSignature) == systemSignature) {
                system->entities.insert(entity);
                if(!contained) {
                    system->onEntityAdded(entity);
                }
            } else {
                system->entities.erase(entity);
                if(contained) {
                    system->onEntityRemoved(entity);
                }
            }
        }
    }

private:
    std::unordered_map<std::size_t, Signature> m_signatures {};
    std::unordered_map<std::size_t, std::unique_ptr<SystemBase>> m_systems {};
};
};  //  namespace emp
#endif
