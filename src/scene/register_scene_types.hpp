#ifndef EMP_REGISTER_SCENE_TYPES_HPP
#define EMP_REGISTER_SCENE_TYPES_HPP
#include "audio/sound_source.hpp"
#include "core/coordinator.hpp"
#include "graphics/animated_sprite.hpp"
#include "graphics/model.hpp"
#include "graphics/particle_system.hpp"
#include "physics/collider.hpp"
#include "physics/constraint.hpp"
#include "physics/material.hpp"
#include "physics/physics_system.hpp"
#include "scene/transform.hpp"
#include "templates/type_pack.hpp"
namespace emp {
struct Device;

typedef TypePack<Transform, Constraint, Material, Collider, Rigidbody, Model, ParticleEmitter, Sprite, AnimatedSprite,
                 SoundSource>
    AllComponentTypes;

void registerSceneTypes(Coordinator &ECS);
void registerSceneSystems(Device &device, Coordinator &ECS);
};
#endif
