#include "rigidbody.hpp"
#include "core/coordinator.hpp"
#include "math/geometry_func.hpp"
#include "math/math_func.hpp"
#include "physics/collider.hpp"
#include <numeric>
namespace emp {
#define SQ(x) ((x) * (x))
Rigidbody::Rigidbody(bool is_static, bool is_rot_locked, bool use_automatic_mass, float density)
    : isStatic(is_static)
    , isRotationLocked(is_rot_locked)
    , useAutomaticMass(use_automatic_mass)
    , real_density(density)
{
}
float Rigidbody::generalizedInverseMass(vec2f radius, vec2f normal) const
{
    if(isStatic) {
        return 0.f;
    }
    return 1.f / mass() + (SQ(perp_dot(radius, normal)) / inertia());
}
void RigidbodySystem::updateMasses()
{
    for(auto entity : entities) {
        auto &rigidobdy = getComponent<Rigidbody>(entity);
        const auto collider = ECS().getComponent<Collider>(entity);
        if(rigidobdy.useAutomaticMass && collider != nullptr) {

            //  auto model =  collider->transformed_outline();
            //  vec2f avg = std::reduce(model.begin(), model.end()) / static_cast<float>(model.size());
            //  for(auto& p : model) { p -= avg; }

            auto MIA = calculateMassInertiaArea(collider->model_outline());
            rigidobdy.real_mass = MIA.area * rigidobdy.real_density;
            rigidobdy.real_inertia = MIA.MMOI * rigidobdy.real_density;
        }
    }
}
void RigidbodySystem::integrate(float delT, float resting_time_threshold)
{
    for(auto entity : entities) {
        auto &rigidbody = getComponent<Rigidbody>(entity);
        if(rigidbody.time_resting > resting_time_threshold || rigidbody.isStatic) {
            continue;
        }
        auto &transform = getComponent<Transform>(entity);
        rigidbody.prev_pos = transform.position;
        rigidbody.velocity += delT * rigidbody.force / rigidbody.mass();
        transform.position += rigidbody.velocity * delT;

        rigidbody.prev_rot = transform.rotation;
        rigidbody.angular_velocity += delT * rigidbody.torque / rigidbody.inertia();
        transform.rotation += rigidbody.angular_velocity * delT;
    }
}
void RigidbodySystem::deriveVelocities(float delT, float resting_time_threshold)
{
    for(auto entity : entities) {
        auto &rigidbody = getComponent<Rigidbody>(entity);
        if(rigidbody.time_resting > resting_time_threshold || rigidbody.isStatic) {
            continue;
        }
        auto &transform = getComponent<Transform>(entity);
        rigidbody.velocity_pre_solve = rigidbody.velocity;
        rigidbody.velocity = (transform.position - rigidbody.prev_pos) / delT;
        rigidbody.ang_velocity_pre_solve = rigidbody.angular_velocity;
        rigidbody.angular_velocity = (transform.rotation - rigidbody.prev_rot) / delT;
    }
}
};  //  namespace emp
