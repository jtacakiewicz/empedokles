#ifndef EMP_HIERARCHY_HPP
#define EMP_HIERARCHY_HPP
#include "core/entity.hpp"
#include "core/system.hpp"
#include <set>
namespace emp {
struct Hierarchy {
    Entity parent;
    std::set<Entity> children;
};
class HierarchySystem : public System { };
};
#endif
