#ifndef EMP_COLLIDER_HPP
#define EMP_COLLIDER_HPP
#include <unordered_set>
#include <vector>
#include "core/layer.hpp"
#include "core/system.hpp"
#include "math/math_defs.hpp"
#include "math/shapes/AABB.hpp"
#include "scene/transform.hpp"
namespace emp {
class ColliderSystem;

struct CollisionInfo {
    Entity collider_entity;
    Entity collidee_entity;
    vec2f relative_velocity;
    float penetration;
    float normal_lagrange;
    vec2f collision_point;
    vec2f collision_normal;
    vec2f collider_radius;
    vec2f collidee_radius;
    void flip();
};
struct Collider {
    typedef std::vector<vec2f> ConvexVertexCloud;
    typedef std::function<void(const CollisionInfo &)> CallbackFunc;

private:
    //  potentially concave
    AABB m_extent;
    std::vector<vec2f> m_model_outline;
    std::vector<ConvexVertexCloud> m_model_shape;

public:
    Layer collider_layer = 0;
    bool isNonMoving = true;

    inline const std::vector<vec2f> &model_outline() const { return m_model_outline; }
    inline const std::vector<ConvexVertexCloud> &model_shape() const { return m_model_shape; }

    std::vector<vec2f> transformed_outline(const Transform &transform) const;
    std::vector<ConvexVertexCloud> transformed_shape(const Transform &transform) const;
    ConvexVertexCloud transformed_convex(const Transform &transform, size_t index) const;

    inline AABB extent() const { return m_extent; }
    Collider() { }
    Collider(std::vector<vec2f> shape, bool correctCOM = false);
    friend ColliderSystem;
};

//  system for updating transfomred collider shapes
class ColliderSystem : public System<Transform, Collider> {
public:
    typedef std::function<void(const CollisionInfo &)> CollisionEnterCallback;
    typedef std::function<void(Entity, Entity)> CollisionExitCallback;

private:
    LayerMask collision_matrix[MAX_LAYERS];
    union FitIntoOne {
        struct {
            //  always lower
            Entity a;
            //  always higher
            Entity b;
        };
        uint64_t hash;
    };
    std::unordered_set<uint64_t> m_collisions_occured_last_frame;
    std::unordered_map<uint64_t, CollisionInfo> m_collisions_occured_this_frame;
    std::unordered_map<Entity, std::vector<CollisionEnterCallback>> m_enter_callbacks;
    std::unordered_map<Entity, std::vector<CollisionExitCallback>> m_exit_callbacks;

    void callAllOnEnterCallbacksFor(Entity e, const CollisionInfo &info);
    void callAllOnExitCallbacksFor(Entity e, Entity other);

public:
    ColliderSystem &onCollisionEnter(Entity listener, Entity target, CollisionEnterCallback &&func);
    ColliderSystem &onCollisionExit(Entity listener, Entity target, CollisionExitCallback &&func);

    void processCollisionNotifications();
    void notifyOfCollision(Entity a, Entity b, CollisionInfo col_info);

    void disableCollision(Layer layer1, Layer layer2);
    void eableCollision(Layer layer1, Layer layer2);
    bool canCollide(Layer layer1, Layer layer2) const;
    void onEntityRemoved(Entity entity) override final;
    ColliderSystem();
};
};  //  namespace emp
#endif
