#ifndef EMP_TRANSFORM_HPP
#define EMP_TRANSFORM_HPP
#include <functional>
#include <vector>
#include "core/coordinator.hpp"
#include "core/system.hpp"
#include "glm/ext/matrix_float4x4.hpp"
#include "math/math_defs.hpp"

namespace emp {
class TransformSystem;
class Transform {
    TransformMatrix m_local_transform;
    TransformMatrix m_parents_global_transform = TransformMatrix(1.f);
    TransformMatrix m_global_transform;

    void m_updateLocalTransform();

    Entity m_parent_entity;
    std::vector<Entity> m_children_entities;

    vec2f m_getPosition(const TransformMatrix &) const;
    vec2f m_getScale(const TransformMatrix &) const;
    float m_getRotation(const TransformMatrix &) const;

public:
    const Entity parent() const { return m_parent_entity; }
    const std::vector<Entity> &children() const { return m_children_entities; }
    vec2f position = vec2f(0.f, 0.f);
    float rotation = 0.f;
    vec2f scale = vec2f(0.f, 0.f);

    void setPositionNow(vec2f p);
    void setRotationNow(float r);
    void setScaleNow(vec2f s);

    vec2f getGlobalPosition();
    vec2f getGlobalScale();
    float getGlobalRotation();

    void syncWithChange();
    inline const TransformMatrix &local() const { return m_local_transform; }
    inline const TransformMatrix &global() const { return m_global_transform; }
    Transform() { }
    Transform(Entity parent, vec2f pos, float rot = 0.f, vec2f s = { 1.f, 1.f })
        : position(pos)
        , rotation(rot)
        , scale(s)
        , m_parent_entity(parent)
    {
        m_updateLocalTransform();
    }
    Transform(vec2f pos, float rot = 0.f, vec2f s = { 1.f, 1.f })
        : Transform(Coordinator::world(), pos, rot, s)
    {
    }

    friend TransformSystem;
};
class TransformSystem : public System<Transform> {
public:
    void performDFS(std::function<void(Entity, Transform &)> &&action);
    void update();
    void onEntityRemoved(Entity entity) override final;
    void onEntityAdded(Entity entity) override final;
};
};  //  namespace emp
#endif
