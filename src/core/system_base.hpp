#ifndef EMP_SYSTEM_BASE_HPP
#define EMP_SYSTEM_BASE_HPP

#include <set>
#include "core/component.hpp"
#include "core/entity.hpp"
namespace emp {
struct SystemManager;
struct Coordinator;
class SystemBase {
public:
    const std::set<Entity> &getEntities() { return entities; }
    virtual void onEntityRemoved(Entity entity) { }
    virtual void onEntityAdded(Entity entity) { }

    SystemBase(const SystemBase &) = delete;
    SystemBase &operator=(const SystemBase &) = delete;
    SystemBase(SystemBase &&) = delete;
    SystemBase &operator=(SystemBase &&) = delete;
    virtual ~SystemBase() { }

    friend SystemManager;

protected:
    Coordinator *coordinator = nullptr;
    inline Coordinator &ECS()
    {
        assert(coordinator != nullptr && "system must be registered via coordinator");
        return *coordinator;
    }
    inline const Coordinator &ECS() const
    {
        assert(coordinator != nullptr && "system must be registered via coordinator");
        return *coordinator;
    }

    std::set<Entity> entities;
    SystemBase() { }
};

template <class... ComponentTypes> class SystemOf : public SystemBase {
protected:
    SystemOf() { }
};
};  //  namespace emp

#endif
