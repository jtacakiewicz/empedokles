#ifndef EMP_SYSTEM_HPP
#define EMP_SYSTEM_HPP
#include "coordinator.hpp"
#include "system_base.hpp"

namespace emp {
template <typename... Components> class System : public SystemOf<Components...> {
    friend Coordinator;
    void setECS(Coordinator *coord) { this->coordinator = coord; }

public:
    template <class T> inline T &getComponent(Entity entity)
    {
        static_assert((std::is_same<T, Components>::value || ...), "must get component contained in this system");
        assert(this->coordinator != nullptr && "system must be registered via coordinator");
        assert(this->entities.contains(entity) && "can only call getComponent on owned entities");
        return *this->ECS().template getComponent<T>(entity);
    }
    template <class T> inline const T &getComponent(Entity entity) const
    {
        static_assert((std::is_same<T, Components>::value || ...), "must get component contained in this system");
        assert(this->coordinator != nullptr && "system must be registered via coordinator");
        assert(this->entities.contains(entity) && "can only call getComponent on owned entities");
        return *this->ECS().template getComponent<T>(entity);
    }
};
struct AllEntitiesSystem : public System<> { };
};  //  namespace emp

#endif
