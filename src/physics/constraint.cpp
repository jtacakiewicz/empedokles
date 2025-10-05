#include "constraint.hpp"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <random>
#include <glm/ext/quaternion_common.hpp>
#include "core/coordinator.hpp"
#include "debug/log.hpp"
#include "math/math_defs.hpp"
#include "math/math_func.hpp"
#include "physics/rigidbody.hpp"
#include "physics_system.hpp"
namespace emp {
PositionalCorrectionInfo::PositionalCorrectionInfo(vec2f normal, Entity e1, vec2f center_to_col1, const Rigidbody *rb1, Entity e2,
                                                   vec2f center_to_col2, const Rigidbody *rb2)
    : entity1(e1)
    , center_to_collision1(center_to_col1)
    , entity2(e2)
    , center_to_collision2(center_to_col2)
{
    if(rb1 == nullptr) {
        EMP_LOG(WARNING) << "no rigidbody to create PositionalCorrectionInfo";
        return;
    }

    isStatic1 = rb1->isStatic;
    inertia1 = rb1->inertia();
    mass1 = rb1->mass();
    generalized_inverse_mass1 = rb1->generalizedInverseMass(center_to_col1, normal);

    if(rb2 != nullptr) {
        isStatic2 = rb2->isStatic;
        inertia2 = rb2->inertia();
        mass2 = rb2->mass();
        generalized_inverse_mass2 = rb2->generalizedInverseMass(center_to_col2, normal);
    } else {
        isStatic2 = true;
        inertia2 = INFINITY;
        mass2 = INFINITY;
        generalized_inverse_mass2 = 0.f;
    }
}
PositionalCorrResult calcPositionalCorrection(PositionalCorrectionInfo info, float c, vec2f normal, float delT, float compliance)
{
    PositionalCorrResult result;
    if(nearlyEqual(c, 0.f)) {
        return result;
    }

    const auto w1 = info.generalized_inverse_mass1;
    const auto w2 = info.generalized_inverse_mass2;

    const auto tilde_compliance = compliance / (delT * delT);

    auto delta_lagrange = -c;
    delta_lagrange /= (w1 + w2 + tilde_compliance);

    auto p = delta_lagrange * normal;

    if(!info.isStatic1) {
        result.pos1_correction = p / info.mass1;
        result.rot1_correction = 0.5 * perp_dot(info.center_to_collision1, p) / info.inertia1;
    }
    if(!info.isStatic2) {
        result.pos2_correction = -p / info.mass2;
        result.rot2_correction = 0.5 * -perp_dot(info.center_to_collision2, p) / info.inertia2;
    }
    result.delta_lagrange = delta_lagrange;

    return result;
}
typedef Constraint::Builder Builder;
Builder &Builder::addConstrainedEntity(Entity entity, const Transform &transform)
{
    entity_list.push_back({ entity, &transform });
    return *this;
}
Builder &Builder::addAnchorEntity(Entity entity, const Transform &transform)
{
    anchor = { entity, &transform };
    return *this;
}

Builder &Builder::setCompliance(float compliance_)
{
    this->compliance = compliance_;
    return *this;
}
Builder &Builder::setDamping(float damping_)
{
    this->damping = damping_;
    return *this;
}

Builder &Builder::enableCollision(bool enable)
{
    this->enabled_collision_between_bodies = enable;
    return *this;
}

Builder &Builder::setFixed(vec2f rel_distance, float rel_rotation)
{
    assert(this->type == eConstraintType::Undefined && "trying to change constraint type");
    type = eConstraintType::FixedLock;
    relative_offset = rel_distance;
    relative_rotation = rel_rotation;
    return *this;
}
Builder &Builder::setHinge(vec2f point)
{
    assert(this->type == eConstraintType::Undefined && "trying to change constraint type");
    type = eConstraintType::SwivelPoint;
    global_point = point;
    return *this;
}
Builder &Builder::setHingeRelative(vec2f rel1, vec2f rel2)
{
    assert(this->type == eConstraintType::Undefined && "trying to change constraint type");
    type = eConstraintType::SwivelPoint;
    point_rel1 = rel1;
    point_rel2 = rel2;
    return *this;
}

Constraint Builder::build()
{
    assert(type != eConstraintType::Undefined && "too little information in constraint builder - no type");
    Constraint result;
    result.damping = damping;
    result.compliance = compliance;
    result.enabled_collision_between_bodies = this->enabled_collision_between_bodies;
    //  conversion of constraint types if anchor set (static object)
    bool usingAnchor = anchor.first != -1 && anchor.second != nullptr;
    if(usingAnchor) {
        switch(type) {
            case eConstraintType::SwivelPoint:
                type = eConstraintType::SwivelPointAnchored;
                break;
            case emp::eConstraintType::FixedLock:
                type = eConstraintType::FixedLockAnchored;
                break;
            case emp::eConstraintType::FixedLockAnchored:
            case emp::eConstraintType::SwivelPointAnchored:
            case emp::eConstraintType::Undefined:
                assert(false && "cannot anchor this constraint");
                break;
        }
        entity_list.insert(entity_list.begin(), anchor);
    }
    for(auto [e, _] : entity_list) {
        result.entity_list.push_back(e);
    }
    result.type = type;
    switch(type) {
        case eConstraintType::SwivelPointAnchored:
            assert(usingAnchor);
            if(!glm::isnan(global_point.x)) {
                point_rel1 = global_point - anchor.second->position;
                point_rel1 = rotate(point_rel1, -anchor.second->rotation);
                point_rel2 = global_point - entity_list.back().second->position;
                point_rel2 = rotate(point_rel2, -entity_list.back().second->rotation);
            }
            result.data.swivel_anchored.anchor_point_model = point_rel1;
            result.data.swivel_anchored.pinch_point_model = point_rel2;
            break;
        case eConstraintType::SwivelPoint:
            assert(!usingAnchor && entity_list.size() == 2);
            if(!glm::isnan(global_point.x)) {
                point_rel1 = global_point - entity_list.front().second->position;
                point_rel1 = rotate(point_rel1, -entity_list.front().second->rotation);
                point_rel2 = global_point - entity_list.back().second->position;
                point_rel2 = rotate(point_rel2, -entity_list.back().second->rotation);
            }
            result.data.swivel_dynamic.pinch_point_model1 = point_rel1;
            result.data.swivel_dynamic.pinch_point_model2 = point_rel2;
            break;
        case eConstraintType::FixedLockAnchored:
            assert(entity_list.size() == 2);
            if(glm::isnan(relative_offset.x)) {
                auto pos1 = entity_list.front().second->position;
                auto pos2 = entity_list.back().second->position;
                relative_offset = rotateVec(pos2 - pos1, -entity_list.front().second->rotation);
            }
            if(glm::isnan(relative_rotation)) {
                auto rot1 = entity_list.front().second->rotation;
                auto rot2 = entity_list.back().second->rotation;
                relative_rotation = rot2 - rot1;
            }

            result.data.fixed_anchored.rel_offset = relative_offset;
            result.data.fixed_anchored.rel_rotation = relative_rotation;
            break;
        case eConstraintType::FixedLock: {
            float base_angle;
            float rel1, rel2;
            if(glm::isnan(relative_offset.x)) {
                auto pos1 = entity_list.front().second->position;
                auto pos2 = entity_list.back().second->position;
                relative_offset = pos2 - pos1;
            } else {
                relative_offset = rotateVec(relative_offset, entity_list.front().second->rotation);
            }
            base_angle = atan2(relative_offset.y, relative_offset.x);
            if(glm::isnan(relative_rotation)) {
                auto rot1 = entity_list.front().second->rotation;
                auto rot2 = entity_list.back().second->rotation;
                rel1 = rot1 - base_angle;
                rel2 = rot2 - base_angle;
            }
            result.data.fixed_dynamic.distance = length(relative_offset);
            result.data.fixed_dynamic.rel_rotation1 = rel1;
            result.data.fixed_dynamic.rel_rotation2 = rel2;
        } break;
        default:
            break;
    }
    return result;
}
void Constraint::m_solvePointSwivel(float delta_time, Coordinator &ECS)
{
    Entity entity1 = entity_list[0];
    Entity entity2 = entity_list[1];
    assert(ECS.hasComponent<Transform>(entity1));
    assert(ECS.hasComponent<Rigidbody>(entity1));
    assert(ECS.hasComponent<Transform>(entity2));
    assert(ECS.hasComponent<Rigidbody>(entity2));

    auto &transform1 = *ECS.getComponent<Transform>(entity1);
    const auto &rigidbody1 = *ECS.getComponent<Rigidbody>(entity1);
    auto &transform2 = *ECS.getComponent<Transform>(entity2);
    const auto &rigidbody2 = *ECS.getComponent<Rigidbody>(entity2);

    const vec2f &pos1 = transform1.position;
    const vec2f &pos2 = transform2.position;

    const auto dynamic_pinch1 = rotate(data.swivel_dynamic.pinch_point_model1, transform1.rotation);
    const auto dynamic_pinch2 = rotate(data.swivel_dynamic.pinch_point_model2, transform2.rotation);
    auto dynamic_point1 = (pos1 + dynamic_pinch1);
    auto dynamic_point2 = (pos2 + dynamic_pinch2);
    auto diff = dynamic_point1 - dynamic_point2;
    auto norm = normal(diff);
    auto c = length(diff);
    if(nearlyEqual(c, 0.f)) {
        return;
    }

    auto correction = calcPositionalCorrection(
        PositionalCorrectionInfo(norm, entity1, dynamic_pinch1, &rigidbody1, entity2, dynamic_pinch2, &rigidbody2), c, norm,
        delta_time, compliance);
    transform1.position += correction.pos1_correction;
    transform1.rotation += correction.rot1_correction;
    transform2.position += correction.pos2_correction;
    transform2.rotation += correction.rot2_correction;
}
void Constraint::m_solvePointFixedAnchor(float delta_time, Coordinator &ECS)
{
    Entity anchor_entity = entity_list[0];
    Entity dynamic_entity = entity_list[1];
    assert(ECS.hasComponent<Transform>(anchor_entity));
    assert(ECS.hasComponent<Transform>(dynamic_entity));
    assert(ECS.hasComponent<Rigidbody>(dynamic_entity));
    const auto &anchor_trans = *ECS.getComponent<Transform>(anchor_entity);
    auto &dynamic_trans = *ECS.getComponent<Transform>(dynamic_entity);
    auto &rigidbody = *ECS.getComponent<Rigidbody>(dynamic_entity);
    const vec2f &pos1 = anchor_trans.position;
    const vec2f &pos2 = dynamic_trans.position;

    vec2f pos_correction = { 0, 0 };
    float rot_correction = 0.f;
    {
        auto pos_diff = pos2 - (pos1 + rotateVec(data.fixed_anchored.rel_offset, anchor_trans.rotation));
        auto norm = normal(pos_diff);
        auto c = length(pos_diff);
        const auto tilde_compliance = compliance / (delta_time * delta_time);

        auto delta_lagrange = -c / (1 + tilde_compliance);
        auto p = delta_lagrange * norm;

        if(!nearlyEqual(c, 0.f)) {
            pos_correction = p;
        }
    }
    {
        auto c = anchor_trans.rotation + data.fixed_anchored.rel_rotation - dynamic_trans.rotation;

        const auto tilde_compliance = compliance / (delta_time * delta_time);

        auto p = c / (1 + tilde_compliance);
        if(!nearlyEqual(c, 0.f)) {
            rot_correction = p;
        }
    }

    dynamic_trans.position += pos_correction;
    if(!rigidbody.isRotationLocked) {
        dynamic_trans.rotation += rot_correction;
    }
}
void Constraint::m_solvePointFixed(float delta_time, Coordinator &ECS)
{
    Entity entity1 = entity_list[0];
    Entity entity2 = entity_list[1];
    assert(ECS.hasComponent<Transform>(entity1));
    assert(ECS.hasComponent<Rigidbody>(entity1));
    assert(ECS.hasComponent<Transform>(entity2));
    assert(ECS.hasComponent<Rigidbody>(entity2));
    auto &trans1 = *ECS.getComponent<Transform>(entity1);
    auto &trans2 = *ECS.getComponent<Transform>(entity2);
    auto &rigidbody1 = *ECS.getComponent<Rigidbody>(entity1);
    auto &rigidbody2 = *ECS.getComponent<Rigidbody>(entity2);
    const vec2f &pos1 = trans1.position;
    const vec2f &pos2 = trans2.position;

    vec2f pos_corr1 = { 0, 0 }, pos_corr2 = { 0, 0 };
    float rot_corr1 = 0, rot_corr2 = 0;

    {
        const auto len = data.fixed_dynamic.distance;
        auto pos_target1 = trans2.position - rotateVec(vec2f(len, 0), trans2.rotation - data.fixed_dynamic.rel_rotation2);
        auto pos_target2 = trans1.position + rotateVec(vec2f(len, 0), trans1.rotation - data.fixed_dynamic.rel_rotation1);

        auto pos1_diff = pos_target1 - trans1.position;
        auto pos2_diff = pos_target2 - trans2.position;
        auto diff = (pos1_diff - pos2_diff) * 0.5f;
        auto c = length(diff);
        auto norm = normal(diff);

        auto correction = calcPositionalCorrection(PositionalCorrectionInfo(norm, entity1, (pos2 - pos1) * 0.5f, &rigidbody1,

                                                                            entity2, (pos1 - pos2) * 0.5f, &rigidbody2),
                                                   -c, norm, delta_time, compliance);
        pos_corr1 = correction.pos1_correction;
        pos_corr2 = correction.pos2_correction;
        rot_corr1 += correction.rot1_correction;
        rot_corr2 += correction.rot2_correction;
    }
    {
        auto target = data.fixed_dynamic.rel_rotation2 - data.fixed_dynamic.rel_rotation1;
        auto c = trans2.rotation - trans1.rotation - target;
        const auto tilde_compliance = compliance * (delta_time * delta_time);
        const auto inertia_sum = rigidbody1.inertia() + rigidbody2.inertia();
        if(!nearlyEqual(c, 0.f)) {
            auto p = c / (2 + tilde_compliance);
            rot_corr1 += p * (rigidbody2.isRotationLocked ? 1.f : rigidbody2.inertia() / inertia_sum);
            rot_corr2 += -p * (rigidbody1.isRotationLocked ? 1.f : rigidbody1.inertia() / inertia_sum);
        }
    }

    trans1.position += pos_corr1;
    trans2.position += pos_corr2;
    trans1.rotation += rot_corr1;
    trans2.rotation += rot_corr2;
}
void Constraint::m_solvePointSwivelAnchor(float delta_time, Coordinator &ECS)
{
    Entity anchor_entity = entity_list[0];
    Entity dynamic_entity = entity_list[1];
    assert(ECS.hasComponent<Transform>(anchor_entity));
    assert(ECS.hasComponent<Transform>(dynamic_entity));
    assert(ECS.hasComponent<Rigidbody>(dynamic_entity));

    const auto &anchor_trans = *ECS.getComponent<Transform>(anchor_entity);
    auto &dynamic_trans = *ECS.getComponent<Transform>(dynamic_entity);
    auto &rigidbody = *ECS.getComponent<Rigidbody>(dynamic_entity);

    const vec2f &pos1 = anchor_trans.position;
    const vec2f &pos2 = dynamic_trans.position;
    auto anchor_point = pos1 + rotate(data.swivel_anchored.anchor_point_model, anchor_trans.rotation);
    auto dynamic_pinch = rotate(data.swivel_anchored.pinch_point_model, dynamic_trans.rotation);
    auto dynamic_point = (pos2 + dynamic_pinch);
    auto diff = dynamic_point - anchor_point;
    auto norm = normal(diff);
    auto c = length(diff);
    if(nearlyEqual(c, 0.f)) {
        return;
    }

    auto correction = calcPositionalCorrection(PositionalCorrectionInfo(norm, dynamic_entity, dynamic_pinch, &rigidbody,

                                                                        anchor_entity, vec2f(0), nullptr),
                                               c, norm, delta_time, compliance);
    dynamic_trans.position += correction.pos1_correction;
    dynamic_trans.rotation += correction.rot1_correction;
}
void Constraint::solve(float delta_time, Coordinator &ECS)
{
    switch(type) {
        case emp::eConstraintType::SwivelPointAnchored:
            m_solvePointSwivelAnchor(delta_time, ECS);
            break;
        case emp::eConstraintType::SwivelPoint:
            m_solvePointSwivel(delta_time, ECS);
            break;
        case emp::eConstraintType::FixedLock:
            m_solvePointFixed(delta_time, ECS);
            break;
        case emp::eConstraintType::FixedLockAnchored:
            m_solvePointFixedAnchor(delta_time, ECS);
            break;
        case emp::eConstraintType::Undefined:
            assert(false);
            break;
    }
}
std::vector<ConstraintSystem::EntityListRef_t> ConstraintSystem::getConstrainedGroups() const
{
    std::vector<ConstraintSystem::EntityListRef_t> result;
    for(auto entity : entities) {
        auto &entity_list = getComponent<Constraint>(entity).entity_list;
        result.push_back(&entity_list);
    }
    return result;
}
void ConstraintSystem::update(float delta_time)
{
    std::vector<Entity> v(entities.begin(), entities.end());
    std::shuffle(v.begin(), v.end(), std::mt19937 { std::random_device {}() });
    for(auto e : v) {
        getComponent<Constraint>(e).solve(delta_time, ECS());
    }
}
};  //  namespace emp
