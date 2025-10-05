#include "transform.hpp"
#include <stack>
#include "core/coordinator.hpp"
#include "debug/log.hpp"
#include "debug/debug.hpp"
#include "glm/ext/matrix_transform.hpp"

namespace emp {
void Transform::m_updateLocalTransform()
{
    m_local_transform = TransformMatrix(1.f);
    m_local_transform = glm::translate(m_local_transform, glm::vec3(position.x, position.y, 0.f));
    m_local_transform = glm::rotate(m_local_transform, rotation, vec3f(0.f, 0.f, 1.f));
    m_local_transform = glm::scale(m_local_transform, vec3f(scale.x, scale.y, 1.f));
}
void Transform::syncWithChange()
{
    m_updateLocalTransform();
    m_global_transform = m_parents_global_transform * m_local_transform;
}
void Transform::setPositionNow(vec2f p)
{
    position = p;
    syncWithChange();
}
void Transform::setRotationNow(float r)
{
    rotation = r;
    syncWithChange();
}
void Transform::setScaleNow(vec2f s)
{
    scale = s;
    syncWithChange();
}
vec2f Transform::m_getPosition(const TransformMatrix &mat) const
{
    return { mat[3][0], mat[3][1] };
}
vec2f Transform::m_getScale(const TransformMatrix &mat) const
{
    vec2f s;
    s.x = glm::length(glm::vec2(mat[0][0], mat[0][1]));  //  Length of column 0
    s.y = glm::length(glm::vec2(mat[1][0], mat[1][1]));  //  Length of column 1
    return s;
}
float Transform::m_getRotation(const TransformMatrix &mat) const
{
    glm::mat4 normalizedmat = mat;
    normalizedmat[0] = glm::vec4(glm::normalize(glm::vec2(normalizedmat[0][0], normalizedmat[0][1])), 0.0f, 0.0f);
    normalizedmat[1] = glm::vec4(glm::normalize(glm::vec2(normalizedmat[1][0], normalizedmat[1][1])), 0.0f, 0.0f);

    //  Extract rotation in radians (from the angle of the first column)
    float r = atan2(normalizedmat[0][1], normalizedmat[0][0]);
    return r;
}
vec2f Transform::getGlobalPosition()
{
    return m_getPosition(m_global_transform);
}
vec2f Transform::getGlobalScale()
{
    return m_getScale(m_global_transform);
}
float Transform::getGlobalRotation()
{
    return m_getRotation(m_global_transform);
}
void TransformSystem::performDFS(std::function<void(Entity, Transform &)> &&action)
{
    std::stack<Entity> to_process;
    to_process.push(Coordinator::world());

    while(!to_process.empty()) {
        auto entity = to_process.top();
        to_process.pop();
        auto &transform = getComponent<Transform>(entity);
        action(entity, transform);

        for(const auto child : transform.children()) {
            auto &childs_trans = getComponent<Transform>(child);
            childs_trans.m_parents_global_transform = transform.global();
            to_process.push(child);
        }
    }
}
void TransformSystem::update()
{
    EMP_DEBUGCALL(bool updated_entities[MAX_ENTITIES] { 0 };)
    this->performDFS([&](Entity entity, Transform &transform) {
        transform.m_updateLocalTransform();
        transform.m_global_transform = transform.m_parents_global_transform * transform.m_local_transform;
        EMP_DEBUGCALL(updated_entities[entity] = true;)
    });

    EMP_DEBUGCALL(for(auto e : entities) {
        if(!updated_entities[e]) {
            EMP_LOG(WARNING) << "didn't updatede transform: " << e << ", because of invalid parent";
        }
    })
}
//  for (auto entity : entities) {
//      auto& trans = getComponent<Transform>(entity);
//      trans.m_parents_global_transform = glm::mat4x4(1.f);
//      trans.m_updateLocalTransform();
//      trans.m_global_transform =
//              trans.m_parents_global_transform * trans.m_local_transform;
//  }
void TransformSystem::onEntityAdded(Entity entity)
{
    if(entity == Coordinator::world()) {
        return;
    }
    auto &transform = getComponent<Transform>(entity);

    const auto parent = transform.parent();
    auto parent_transform = ECS().getComponent<Transform>(parent);

    if(parent_transform == nullptr) {
        EMP_LOG(WARNING) << "assigned a parent without transform, reassigning to world";
        transform.m_parent_entity = Coordinator::world();
        if(ECS().hasComponent<Transform>(Coordinator::world())) {
            parent_transform = ECS().getComponent<Transform>(Coordinator::world());
        }
    }
    parent_transform->m_children_entities.push_back(entity);
}
void TransformSystem::onEntityRemoved(Entity entity)
{
    auto &transform = getComponent<Transform>(entity);
    const auto parent = transform.parent();
    if(!ECS().isEntityAlive(parent)) {
        return;
    }
    auto parent_transform = ECS().getComponent<Transform>(parent);
    if(parent_transform == nullptr) {
        EMP_LOG(WARNING) << "parent without transfrom, but had when assigning";
        return;
    }
    auto &children = parent_transform->m_children_entities;
    auto itr = std::find(children.begin(), children.end(), entity);
    if(itr != children.end()) {
        children.erase(itr);
    }
}
};  //  namespace emp
