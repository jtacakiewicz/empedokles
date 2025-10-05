#ifndef EMP_RIGIDBODY_HPP
#define EMP_RIGIDBODY_HPP
#include "math/math_defs.hpp"
#include "scene/transform.hpp"
namespace emp {
struct RigidbodySystem;
class Rigidbody {

    vec2f prev_pos = vec2f(0.f, 0.f);
    vec2f velocity_pre_solve = vec2f(0.f, 0.f);

    float prev_rot = 0.f;
    float ang_velocity_pre_solve = 0.f;

public:
    float real_inertia = 1.f;
    float real_mass = 1.f;
    float real_density = 1.f;

    bool isStatic = false;
    bool isRotationLocked = false;

    float time_resting = 0.f;
    bool useAutomaticMass = true;

    vec2f velocity = vec2f(0.f, 0.f);
    vec2f force = vec2f(0.f, 0.f);
    float angular_velocity = 0.f;
    float torque = 0.f;

    inline vec2f previous_position() const { return prev_pos; }
    inline float previous_rotation() const { return prev_rot; }
    inline vec2f previous_velocity() const { return isStatic ? vec2f(0) : velocity_pre_solve; }
    inline float previous_angular_velocity() const { return isStatic ? 0.f : ang_velocity_pre_solve; }

    float inertia() const { return (isStatic || isRotationLocked) ? INFINITY : real_inertia; }
    float mass() const { return isStatic ? INFINITY : real_mass; }
    float generalizedInverseMass(vec2f radius, vec2f normal) const;

    Rigidbody() { }
    Rigidbody(bool is_static, bool is_rot_locked = false, bool use_automatic_mass = true, float density = 1.f);
    friend RigidbodySystem;
};
class RigidbodySystem : public System<Transform, Rigidbody> {
public:
    //  if resting_time in rigidbody is bigger than threshold it is considered not moving
    void integrate(float delT, float restingTimeThreshold = INFINITY);
    void deriveVelocities(float delT, float restingTimeThreshold = INFINITY);
    void updateMasses();
};
};  //  namespace emp
#endif
