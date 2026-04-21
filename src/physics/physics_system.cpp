#include "physics_system.hpp"
#include <glm/vector_relational.hpp>
#include <memory>
#include "core/coordinator.hpp"
#include "debug/log.hpp"
#include "math/geometry_func.hpp"
#include "math/math_func.hpp"
#include "physics/collider.hpp"
#include "physics/constraint.hpp"
#include "physics/rigidbody.hpp"

#include "templates/sweep_line.hpp"
#include "utils/time.hpp"
namespace emp {
float PhysicsSystem::m_calcRestitution(float coef, float normal_speed, float pre_solve_norm_speed, vec2f gravity, float delT)
{
    if(pre_solve_norm_speed >= 0.0f) {
        return 0.0f;
    }

    float target_v_change = -normal_speed - (coef * pre_solve_norm_speed);

    return fmax(target_v_change, 0.0f);
}
float PhysicsSystem::m_calcDynamicFriction(float coef, float tangent_speed, float generalized_inv_mass_sum, float normal_lagrange,
                                           float sub_dt)
{
    auto normal_impulse = normal_lagrange / sub_dt;
    return fmin((coef * abs(normal_impulse)), tangent_speed / generalized_inv_mass_sum);
}
vec2f PhysicsSystem::m_calcContactVel(vec2f vel, float ang_vel, vec2f r)
{
    return vel + ang_vel * vec2f(-r.y, r.x);
}
PhysicsSystem::PenetrationConstraint PhysicsSystem::m_detectCollision(Entity e1, const int convexIdx1, Entity e2,
                                                                      const int convexIdx2, float delT, float compliance)
{
    PenetrationConstraint result;
    result.info.collider_entity = e1;
    result.info.collidee_entity = e2;

    const auto &trans1 = getComponent<Transform>(e1);
    const auto &col1 = getComponent<Collider>(e1);

    const auto &trans2 = getComponent<Transform>(e2);
    const auto &col2 = getComponent<Collider>(e2);

    const vec2f pos1 = trans1.position;
    const vec2f pos2 = trans2.position;
    const float rot1 = trans1.rotation;
    const float rot2 = trans2.rotation;

    auto intersectingShape1 = col1.transformed_shape(trans1)[convexIdx1];
    auto intersectingShape2 = col2.transformed_shape(trans2)[convexIdx2];
    auto intersection = intersectPolygonPolygon(intersectingShape1, intersectingShape2);
    result.detected = intersection.detected;
    auto normal = -intersection.contact_normal;
    const auto penetration = intersection.overlap;
    auto p1 = intersection.cp1;
    auto p2 = intersection.cp2;

    result.info.collision_normal = intersection.contact_normal;
    result.info.penetration = penetration;

    const auto center_to_col_point1 = p1 - pos1;
    const auto center_to_col_point2 = p2 - pos2;
    const auto radius1 = rotateVec(center_to_col_point1, -rot1);
    const auto radius2 = rotateVec(center_to_col_point2, -rot2);
    result.info.collider_radius = radius1;
    result.info.collidee_radius = radius2;

    return result;
}
void PhysicsSystem::m_handleCollision(PhysicsSystem::PenetrationConstraint &constraint, Entity e1, Entity e2, float delT,
                                      float compliance)
{
    assert(constraint.info.detected);
    auto &trans1 = getComponent<Transform>(e1);
    auto &rb1 = getComponent<Rigidbody>(e1);
    auto &mat1 = getComponent<Material>(e1);

    auto &trans2 = getComponent<Transform>(e2);
    auto &rb2 = getComponent<Rigidbody>(e2);
    auto &mat2 = getComponent<Material>(e2);

    vec2f &pos1 = trans1.position;
    vec2f &pos2 = trans2.position;
    float &rot1 = trans1.rotation;
    float &rot2 = trans2.rotation;

    const float mass1 = rb1.mass();
    const float mass2 = rb2.mass();
    const float inertia1 = rb1.inertia();
    const float inertia2 = rb2.inertia();

    const float sfriction = 0.5f * (mat1.static_friction + mat2.static_friction);
    const float dfriction = 0.5f * (mat1.dynamic_friction + mat2.dynamic_friction);
    const float restitution = 0.5f * (mat1.restitution + mat2.restitution);

    constraint.isStatic1 = rb1.isStatic;
    constraint.isStatic2 = rb2.isStatic;

    constraint.sfriction = sfriction;
    constraint.dfriction = dfriction;
    constraint.restitution = restitution;

    const auto penetration = constraint.info.penetration;

    const auto radius1 = constraint.info.collider_radius;
    const auto radius2 = constraint.info.collidee_radius;
    const auto center_to_col_point1 = rotateVec(radius1, rot1);
    const auto center_to_col_point2 = rotateVec(radius2, rot2);
    const auto normal = -constraint.info.collision_normal;

    auto penetration_correction =
        calcPositionalCorrection(PositionalCorrectionInfo(normal, e1, center_to_col_point1, &rb1, e2, center_to_col_point2, &rb2),
                                 penetration, normal, delT);
    pos1 += penetration_correction.pos1_correction;
    rot1 += penetration_correction.rot1_correction;
    pos2 += penetration_correction.pos2_correction;
    rot2 += penetration_correction.rot2_correction;

    float delta_lagrange = penetration_correction.delta_lagrange;

    constraint.info.normal_lagrange = delta_lagrange;

    auto displacementOfPoint = [](vec2f pos, float rot, vec2f radius, Rigidbody &rb) {
        if(rb.isStatic) {
            return vec2f(0, 0);
        }
        return pos - rb.previous_position() +
               (rb.isRotationLocked ? vec2f(0) : rotateVec(radius, rot) - rotateVec(radius, rb.previous_rotation()));
    };

    auto delta_p1 = displacementOfPoint(pos1, rot1, radius1, rb1);
    auto delta_p2 = displacementOfPoint(pos2, rot2, radius2, rb2);
    auto delta_p = delta_p1 - delta_p2;
    auto delta_p_tangent = delta_p - dot(delta_p, normal) * normal;
    auto sliding_len = length(delta_p_tangent);

    if(sliding_len <= 0.f) {
        trans1.syncWithChange();
        trans2.syncWithChange();
        return;
    }
    auto tangent = delta_p_tangent / sliding_len;

    static const float sliding_tolerance = 0.1f;
    if(sliding_len <= sfriction * penetration ||
       nearlyEqual(sfriction * penetration - sliding_len, 0.f, sliding_tolerance * delT)) {
        auto friction_correction = calcPositionalCorrection(
            PositionalCorrectionInfo(tangent, e1, center_to_col_point1, &rb1, e2, center_to_col_point2, &rb2), sliding_len,
            tangent, delT, 0.f);
        pos1 += friction_correction.pos1_correction;
        rot1 += friction_correction.rot1_correction;
        pos2 += friction_correction.pos2_correction;
        rot2 += friction_correction.rot2_correction;

        delta_lagrange = friction_correction.delta_lagrange;
    }
    trans1.syncWithChange();
    trans2.syncWithChange();
    return;
}
bool PhysicsSystem::m_isCollisionAllowed(const CollidingPair &pair, const ColliderSystem &col_sys) const
{
    auto e1 = std::get<Entity>(pair.first);
    auto e2 = std::get<Entity>(pair.second);
    auto s1i = std::get<size_t>(pair.first);
    auto s2i = std::get<size_t>(pair.second);
    const auto &col1 = getComponent<Collider>(e1);
    const auto &col2 = getComponent<Collider>(e2);
    const auto &rb1 = getComponent<Rigidbody>(e1);
    const auto &rb2 = getComponent<Rigidbody>(e2);
    if(e1 == e2) {
        return false;
    }
    if(col1.isNonMoving && col2.isNonMoving) {
        return false;
    }
    if(rb1.isStatic && rb2.isStatic) {
        return false;
    }
    if(!col_sys.canCollide(col1.collider_layer, col2.collider_layer)) {
        return false;
    }
    return true;
}
void PhysicsSystem::m_filterPotentialCollisions(std::vector<CollidingPair> &pairs, const ColliderSystem &col_sys)
{
    for(int i = 0; i < pairs.size(); i++) {
        if(!m_isCollisionAllowed(pairs[i], col_sys)) {
            pairs.erase(pairs.begin() + i);
            i--;
            continue;
        }
    }
}
void PhysicsSystem::m_updateQuadTree()
{
    AABB current_minimal = AABB::Expandable();
    for(auto e : entities) {
        const auto &col = getComponent<Collider>(e);
        const auto &trans = getComponent<Transform>(e);

        auto aabb = col.extent();
        aabb = AABB::TransformedAABB(trans.global(), aabb);

        current_minimal.expandToContain(aabb.min);
        current_minimal.expandToContain(aabb.max);
    }
    if(m_quad_tree == nullptr || !AABBcontainsAABB(m_quad_tree->getAABB(), current_minimal)) {
        if(m_quad_tree) {
            EMP_LOG(DEBUG2) << "quad tree rebuilt, size: " << m_quad_tree->getAABB().size().x << "\t"
                            << m_quad_tree->getAABB().size().y;
        }
        m_quad_tree.reset();
        current_minimal.setSize(current_minimal.size() * 2.f);
        m_quad_tree = std::unique_ptr<QuadTree_t>(new QuadTree_t(current_minimal, m_aabb_extracter));
    }
    m_quad_tree->clear();
    for(auto e : entities) {
        const auto &col = getComponent<Collider>(e);
        const auto &trans = getComponent<Transform>(e);
        auto shape = col.transformed_shape(trans);
        for(int i = 0; i < shape.size(); i++) {
            auto aabb = AABB::CreateFromVerticies(shape[i]);
            aabb.setSize(aabb.size() * 1.5f);
            m_quad_tree->add({ e, i, aabb });
        }
    }
    m_quad_tree->updateLeafes();
}
std::vector<CollidingPair> PhysicsSystem::m_broadPhase(const ColliderSystem &collider_system,
                                                       const TransformSystem &transform_system)
{
    auto all_pairs = m_quad_tree->findAllIntersections();
    m_filterPotentialCollisions(all_pairs, collider_system);
    return all_pairs;
}
std::vector<PhysicsSystem::PenetrationConstraint>
PhysicsSystem::m_narrowPhase(ColliderSystem &col_sys, const std::vector<CollidingPair> &pairs, float delT)
{
    std::vector<PenetrationConstraint> result(pairs.size());

    for(size_t i = 0; i < pairs.size(); ++i) {
        const auto &pair = pairs[i];

        auto e1 = std::get<Entity>(pair.first);
        auto e2 = std::get<Entity>(pair.second);
        auto s1i = std::get<size_t>(pair.first);
        auto s2i = std::get<size_t>(pair.second);
        auto res = m_detectCollision(e1, s1i, e2, s2i, delT);
        if(!res.detected) {
            result[i].detected = false;
            continue;
        }
        result[i] = res;
    }
    for(size_t i = 0; i < pairs.size(); ++i) {
        auto &res = result[i];
        if(!res.detected) {
            continue;
        }

        const auto &pair = pairs[i];
        auto e1 = std::get<Entity>(pair.first);
        auto e2 = std::get<Entity>(pair.second);
        m_handleCollision(res, e1, e2, delT);

        col_sys.notifyOfCollision(e1, e2, res.info);

        if(!res.isStatic1 && !res.isStatic2) {
            m_have_collided.set(e1);
            m_have_collided.set(e2);
            m_collision_islands.merge(e1, e2);
        }
    }
    return result;
}
void PhysicsSystem::m_broadcastCollisionMessages(const std::vector<PenetrationConstraint> &constraints)
{
    for(const auto &constraint : constraints) {
        if(!constraint.detected) {
            continue;
        }
        auto col_info = constraint.info;
        auto &col1 = getComponent<Collider>(col_info.collider_entity);
        auto &col2 = getComponent<Collider>(col_info.collidee_entity);
        //  col1.broadcastCollision(col_info);

        col_info.collision_normal *= -1.f;
        col_info.relative_velocity *= -1.f;
        std::swap(col_info.collider_entity, col_info.collidee_entity);
        std::swap(col_info.collider_radius, col_info.collidee_radius);
        //  col2.broadcastCollision(col_info);
    }
}
//  need to update colliders after
void PhysicsSystem::m_solveVelocities(std::vector<PenetrationConstraint> &constraints, float delT)
{
    for(auto &constraint : constraints) {
        if(!constraint.detected) {
            continue;
        }
        const auto e1 = constraint.info.collider_entity;
        const auto e2 = constraint.info.collidee_entity;

        const auto &trans1 = getComponent<Transform>(e1);
        auto &rb1 = getComponent<Rigidbody>(e1);

        const auto &trans2 = getComponent<Transform>(e2);
        auto &rb2 = getComponent<Rigidbody>(e2);

        const auto restitution = constraint.restitution;

        const auto r1model = rotateVec(constraint.info.collider_radius, trans1.rotation);
        const auto r2model = rotateVec(constraint.info.collidee_radius, trans2.rotation);

        const auto pre_solve_contact_vel1 = m_calcContactVel(rb1.previous_velocity(), rb1.previous_angular_velocity(), r1model);
        const auto pre_solve_contact_vel2 = m_calcContactVel(rb2.previous_velocity(), rb2.previous_angular_velocity(), r2model);
        const auto pre_solve_relative_vel = pre_solve_contact_vel1 - pre_solve_contact_vel2;
        const auto pre_solve_normal_speed = dot(pre_solve_relative_vel, constraint.info.collision_normal);

        const auto contact_vel1 = m_calcContactVel(rb1.velocity, rb1.angular_velocity, r1model);
        const auto contact_vel2 = m_calcContactVel(rb2.velocity, rb2.angular_velocity, r2model);
        const auto relative_vel = contact_vel1 - contact_vel2;
        constraint.info.relative_velocity = relative_vel;
        const auto normal_speed = dot(relative_vel, constraint.info.collision_normal);

        const auto tangent_vel = relative_vel - constraint.info.collision_normal * normal_speed;
        const auto tangent_speed = length(tangent_vel);

        vec2f p = { 0, 0 };
        auto restitution_speed = m_calcRestitution(restitution, normal_speed, pre_solve_normal_speed, gravity, delT);
        if(abs(restitution_speed) > 0.f) {
            const auto w1 = rb1.generalizedInverseMass(r1model, constraint.info.collision_normal);
            const auto w2 = rb2.generalizedInverseMass(r2model, constraint.info.collision_normal);
            const auto restitution_impulse = restitution_speed / (w1 + w2);
            p += restitution_impulse * constraint.info.collision_normal;
        }

        const float sfriction = constraint.sfriction;
        const float dfriction = constraint.dfriction;
        //  Compute dynamic friction
        if(abs(tangent_speed) > 0.f) {
            const auto tangent_normal = tangent_vel / tangent_speed;
            const auto w1 = rb1.generalizedInverseMass(r1model, tangent_normal);
            const auto w2 = rb2.generalizedInverseMass(r2model, tangent_normal);
            auto friction_impulse =
                m_calcDynamicFriction(dfriction, tangent_speed, w1 + w2, constraint.info.normal_lagrange, delT);
            p -= tangent_normal * friction_impulse;
        }
        if(!rb1.isStatic) {
            const auto delta_lin_vel = p / rb1.mass();
            const auto delta_ang_vel = perp_dot(r1model, p) / rb1.inertia();
            rb1.velocity += delta_lin_vel;
            rb1.angular_velocity += delta_ang_vel;
        }
        if(!rb2.isStatic) {
            const auto delta_lin_vel = p / rb2.mass();
            const auto delta_ang_vel = perp_dot(r2model, p) / rb2.inertia();
            rb2.velocity -= delta_lin_vel;
            rb2.angular_velocity -= delta_ang_vel;
        }
    }
}
void PhysicsSystem::m_applyGravity(float delta_time)
{
    for(const auto e : entities) {
        auto &rb = getComponent<Rigidbody>(e);
        if(m_isDormant(rb)) {
            continue;
        }
        if(!rb.isStatic) {
            rb.force += gravity * rb.mass();
        }
    }
}
void PhysicsSystem::m_applyAirDrag(float delta_time)
{
    for(const auto e : entities) {
        auto &rb = getComponent<Rigidbody>(e);
        if(m_isDormant(rb)) {
            continue;
        }
        auto &material = getComponent<Material>(e);
        auto magnitude = dot(rb.velocity, rb.velocity);
        if(magnitude == 0.f) {
            continue;
        }
        auto direction = -normal(rb.velocity);
        if(!rb.isStatic) {
            rb.force += direction * magnitude * material.air_friction * delta_time;
        }
    }
}
void PhysicsSystem::m_processSleepingGroups(float delta_time)
{
    for(const auto e : entities) {
        auto &rb = getComponent<Rigidbody>(e);
        if(rb.isStatic) {
            continue;
        }
        auto head = m_collision_islands.group(e);
        auto &head_rb = getComponent<Rigidbody>(head);
        if(length(rb.velocity) > SLOW_VEL) {
            head_rb.time_resting = 0.f;
        }
    }
    for(const auto e : entities) {
        auto &rb = getComponent<Rigidbody>(e);
        if(rb.isStatic) {
            continue;
        }
        auto head = m_collision_islands.group(e);
        auto &head_rb = getComponent<Rigidbody>(head);
        if(head_rb.time_resting == 0.f) {
            rb.time_resting = 0.f;
            m_collision_islands.isolate(e);
        }
    }
    for(const auto e : entities) {
        auto &rb = getComponent<Rigidbody>(e);
        if(rb.isStatic) {
            continue;
        }
        rb.time_resting += delta_time;
    }
}
void PhysicsSystem::m_mergeConstrainedSleepingGroups(ConstraintSystem &constr_sys)
{
    auto all_groups = constr_sys.getConstrainedGroups();
    for(const auto &group : all_groups) {
        auto prev = group->back();
        for(auto entity : *group) {
            m_collision_islands.merge(entity, prev);
        }
    }
}
void PhysicsSystem::m_processSleep(float delta_time, ConstraintSystem &constr_sys)
{
    m_mergeConstrainedSleepingGroups(constr_sys);
    m_processSleepingGroups(delta_time);
    for(const auto e : entities) {
        auto &rb = getComponent<Rigidbody>(e);
        auto &col = getComponent<Collider>(e);
        if(rb.isStatic) {
            continue;
        }
        col.isNonMoving = m_isDormant(rb);
    }
}
bool PhysicsSystem::m_isDormant(const Rigidbody &rb) const
{
    return useDeactivation && rb.time_resting > DORMANT_TIME_THRESHOLD;
}
void PhysicsSystem::m_step(TransformSystem &trans_sys, ColliderSystem &col_sys, RigidbodySystem &rb_sys,
                           ConstraintSystem &const_sys, float delta_time)
{
    m_processSleep(delta_time, const_sys);
    rb_sys.integrate(delta_time, DORMANT_TIME_THRESHOLD);
    trans_sys.update();
    const_sys.update(delta_time);
    auto potential_pairs = m_broadPhase(col_sys, trans_sys);
    auto penetrations = m_narrowPhase(col_sys, potential_pairs, delta_time);

    trans_sys.update();
    rb_sys.deriveVelocities(delta_time, DORMANT_TIME_THRESHOLD);
    m_solveVelocities(penetrations, delta_time);
    m_broadcastCollisionMessages(penetrations);
}
void PhysicsSystem::update(TransformSystem &trans_sys, ColliderSystem &col_sys, RigidbodySystem &rb_sys,
                           ConstraintSystem &const_sys, float delT)
{
    Stopwatch stop;
    m_have_collided.reset();
    trans_sys.update();
    m_updateQuadTree();
    m_applyGravity(delT);
    m_applyAirDrag(delT);
    for(int i = 0; i < substep_count; i++) {
        m_step(trans_sys, col_sys, rb_sys, const_sys, delT / static_cast<float>(substep_count));
        for(const auto e : entities) {
            auto &rb = getComponent<Rigidbody>(e);
            rb.force = { 0, 0 };
            rb.torque = 0.f;
        }
    }
    col_sys.processCollisionNotifications();
    m_separateNonColliding();
}
void PhysicsSystem::m_separateNonColliding()
{
    for(auto e : entities) {
        auto &rb = getComponent<Rigidbody>(e);
        if(!m_have_collided.test(e) && !m_isDormant(rb)) {
            m_collision_islands.isolate(e);
        }
    }
}
};  //  namespace emp
