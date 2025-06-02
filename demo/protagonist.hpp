#pragma once
#include "core/coordinator.hpp"
#include "graphics/animated_sprite.hpp"
#include "gui/gui_manager.hpp"
#include "math/math_defs.hpp"
#include "physics/rigidbody.hpp"
namespace emp {
struct Protagonist {
    Entity id;
    std::vector<vec2f> shape = {
            vec2f(-100.f/4, -100.f/1.5),
            vec2f(-100.f/4, 100.f/1.5),
            vec2f(100.f/4, 100.f/1.5),
            vec2f(100.f/4, -100.f/1.5)
    };
    
    float speed = 600.f;
    float speed_max= 100000.f;
    Entity isGrounded;
    bool isAttacking;
    bool isRolling;
    float isGroundedSec;
    void setup(Coordinator& ECS, GUIManager& gui_manager) {
        id = ECS.createEntity();
        gui_manager.alias(id, "protagonist");
        setupPhysics(ECS);
        setupAnimation(ECS);
    }
    void setupPhysics(Coordinator& ECS) {

        ECS.getSystem<ColliderSystem>()
            ->onCollisionEnter(id,
                id,
                [&](const CollisionInfo& info) {
                    auto ang = angle(info.collision_normal, vec2f(0, 1));
                    if (cos(ang) > -M_PI / 4.f) {
                        return;
                    }
                    isGrounded = info.collidee_entity;
                })
            .onCollisionExit(
                id, id, [&](Entity me, Entity other) {
                    if (other == isGrounded || isGrounded == false) {
                        isGrounded = false;
                    }
                });
        Rigidbody rb;
        rb.useAutomaticMass = true;
        rb.isRotationLocked = true;
        ECS.addComponent(id, Transform(vec2f(), 0.f));
        auto col = Collider(shape);
        col.collider_layer = 1;

        ECS.addComponent(id, col);
        ECS.addComponent(id, rb);

        auto mat = Material(); 
        ECS.addComponent(id, mat);

        // auto child = ECS.createEntity();
        // gui_manager.alias(child, "child");
        // ECS.addComponent(child, Transform(id, vec2f(0, 0)));
    }
    void setupAnimation(Coordinator& ECS) {
        auto def_size = vec2f(300.f, 300.f);
        auto offset = vec2f(10.f, -def_size.y / 3.5f);
        MovingSprite idle_moving;
        {
            Sprite idle_sprite = Sprite(Texture("idle"), def_size);
            idle_sprite.centered = true;
            idle_sprite.hframes = 10;
            idle_sprite.vframes = 1;
            idle_moving.sprite = idle_sprite;
            for(int i = 0; i < idle_sprite.frameCount(); i++) {
                idle_moving.add(i, 0.085f);
            }
        }
        AnimatedSprite::Builder build("idle", idle_moving);
        {
            {
                Sprite jumping_spr = Sprite(Texture("attack"), def_size);
                jumping_spr.centered = true;
                jumping_spr.hframes = 4;
                jumping_spr.vframes = 1;
                build.addNode("attack", MovingSprite::allFrames(jumping_spr, 0.30f, false));
            }
            {
                Sprite jumping_spr = Sprite(Texture("roll"), def_size);
                jumping_spr.centered = true;
                jumping_spr.hframes = 12;
                jumping_spr.vframes = 1;
                build.addNode("roll", MovingSprite::allFrames(jumping_spr, 0.30f, false));
            }
            {
                Sprite running_sprite = Sprite(Texture("running"), def_size);
                running_sprite.centered = true;
                running_sprite.hframes = 10;
                running_sprite.vframes = 1;
                build.addNode("run",  MovingSprite::allFrames(running_sprite, 0.6f));
            }
            {
                Sprite jumping_spr = Sprite(Texture("jump-up"), def_size);
                jumping_spr.centered = true;
                jumping_spr.hframes = 3;
                jumping_spr.vframes = 1;
                build.addNode("jump", MovingSprite::allFrames(jumping_spr, 0.25f, false));
            }
            {
                Sprite jumpfall_spr = Sprite(Texture("jumpfall"), def_size);
                jumpfall_spr.centered = true;
                jumpfall_spr.hframes = 2;
                jumpfall_spr.vframes = 1;
                build.addNode("jumpfall", MovingSprite::allFrames(jumpfall_spr, 0.25f, false));
            }
            {
                Sprite falling_spr = Sprite(Texture("jump-down"), def_size);
                falling_spr.centered = true;
                falling_spr.hframes = 3;
                falling_spr.vframes = 1;
                build.addNode("fall", MovingSprite::allFrames(falling_spr, 0.25f, false));
            }

            build.addEdge("idle", "run", [this, &ECS](Entity owner) {
                return abs(ECS.getComponent<Rigidbody>(owner)->velocity.x) >
                       25.f;
            });
            build.addEdge("run", "idle", [&, this](Entity owner, bool isended) {
                return abs(ECS.getComponent<Rigidbody>(owner)->velocity.x) <
                       25.f;
            });
            auto isJumping = [&, this](Entity owner) {
                return ECS.getComponent<Rigidbody>(owner)->velocity.y <
                       -100.f;
            };
            auto isFalling = [&, this](Entity owner) {
                return ECS.getComponent<Rigidbody>(owner)->velocity.y >
                       25.f;
            };
            auto hasFallen = [&](Entity owner) {
                return isGrounded != false;
            };
            auto isAttackingCh = [&](Entity owner) {
                return isAttacking;
            };
            auto isRollingCh = [&](Entity owner) {
                return isRolling;
            };
            build.addEdge("idle", "attack", isAttackingCh);
            build.addEdge("run", "attack", isAttackingCh);
            build.addEdge("attack", "idle");

            build.addEdge("run", "roll", isRollingCh);
            build.addEdge("roll", "run");

            build.addEdge("idle", "fall", isFalling);
            build.addEdge("run", "fall", isFalling);
            build.addEdge("idle", "jump", isJumping);
            build.addEdge("run", "jump", isJumping);
            build.addEdge("jump", "jumpfall", isFalling);
            build.addEdge("jumpfall", "fall");
            build.addEdge("fall", "idle", hasFallen);
        }
        {
            std::array<Vertex, 4U> verts;
            for(int i = 0; i < verts.size(); i++) {
                verts[i].color = {1, 0, 0};
                verts[i].uv = {0, 0};

                auto v = shape[i];
                verts[i].position = vec3f(v.x, v.y, 0.f);
            }
        }
        auto anim_sprite = AnimatedSprite(build);
        anim_sprite.position_offset = offset;
        ECS.addComponent(id, anim_sprite);
    }
    
};
};
