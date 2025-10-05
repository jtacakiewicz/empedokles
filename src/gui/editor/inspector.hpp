#ifndef EMP_INSPECTOR_HPP
#define EMP_INSPECTOR_HPP
#include "imgui.h"
#include <glm/fwd.hpp>
#include <cstdint>
#include "core/coordinator.hpp"
#include "core/entity.hpp"
#include "debug/log.hpp"
#include "scene/register_scene_types.hpp"
#include "templates/type_pack.hpp"
namespace emp {
namespace helper {
    template <class T> std::string prettyName()
    {
        std::string result(typeid(T).name());
        int found = -1;
        for(char ch = '0'; ch <= '9'; ch++) {
            found = std::max(found, (int)result.rfind(ch));
        }
        if(found == -1) {
            return result;
        }
        return result.substr(found + 1);
    }
    template <class Vec> float *VecToPtr(Vec &vec)
    {
        return &vec.x;
    }
    template <class MatrixT> void displayMat(MatrixT mat, const char *label = "unnamed")
    {
        ImGui::BeginGroup();

        ImGui::Text("%s", label);
        if(ImGui::BeginTable(label, 4, ImGuiTableFlags_Borders | ImGuiTableFlags_NoHostExtendX)) {
            ImGui::TableSetupColumn("Column 1", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Column 2", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Column 3", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Column 4", ImGuiTableColumnFlags_WidthFixed);
            for(int row = 0; row < 4; ++row) {
                ImGui::TableNextRow();
                for(int col = 0; col < 4; ++col) {
                    ImGui::TableSetColumnIndex(col);
                    ImGui::Text("% .3f", mat[row][col]);
                }
            }
            ImGui::EndTable();
        }
        ImGui::EndGroup();
    }
};
class Inspector {
    template <class T> struct Inspect {
        void operator()(Entity e, T &component)
        {
            EMP_LOG_INTERVAL(WARNING, 1.f) << "inspection not defined for: " << helper::prettyName<T>();
        }
    };
    template <class CompType> void inspectProxy(Entity e, Coordinator &ECS)
    {
        auto *comp = ECS.getComponent<CompType>(e);
        if(comp == nullptr) {
            return;
        }
        auto header_name = helper::prettyName<CompType>();
        if(ImGui::CollapsingHeader(header_name.c_str())) {
            ImGui::Indent();
            Inspect<CompType>()(e, *comp);
            ImGui::Unindent();
        }
    }

    template <class... Ts> void inspectAll(Entity e, Coordinator &ECS, TypePack<Ts...>) { (inspectProxy<Ts>(e, ECS), ...); }

public:
    bool isOpen = true;
    void draw(Entity e, Coordinator &ECS)
    {
        if(!isOpen) {
            return;
        }
        ImGui::Begin("Entity Inspector", &isOpen, ImGuiWindowFlags_AlwaysVerticalScrollbar);
        inspectAll(e, ECS, AllComponentTypes());
        ImGui::End();
    }
};
template <> struct Inspector::Inspect<Transform> {
    void operator()(Entity e, Transform &transform)
    {
        ImGui::DragFloat2("position", helper::VecToPtr(transform.position));
        constexpr float max_rotation = M_PI * 2.f;
        ImGui::DragFloat("rotation", &transform.rotation, 0.01f, -max_rotation, max_rotation);
        ImGui::DragFloat2("scale", helper::VecToPtr(transform.scale), 0.1f);
        helper::displayMat(transform.local(), "local transform");
        ImGui::SameLine();
        helper::displayMat(transform.global(), "global transform");
    }
};
template <> struct Inspector::Inspect<Material> {
    void operator()(Entity e, Material &material)
    {
        ImGui::DragFloat("restitution", &material.restitution, 0.1f);
        ImGui::DragFloat("static_fric ", &material.static_friction, 0.01f, 0.f, 1.f);
        ImGui::DragFloat("dynamic_fric", &material.dynamic_friction, 0.01f, 0.f, 1.f);
        ImGui::DragFloat("air_friction", &material.air_friction, 0.1f, 0.f, INFINITY);
    }
};

template <> struct Inspector::Inspect<Collider> {
    void operator()(Entity e, Collider &collider)
    {
        ImGui::Checkbox("isNonMoving", &collider.isNonMoving);
        const uint8_t min_layers = 0;
        const uint8_t max_layers = MAX_LAYERS - 1;
        ImGui::SliderScalar("collider layer", ImGuiDataType_U8, &collider.collider_layer, &min_layers, &max_layers);
    }
};
template <> struct Inspector::Inspect<Rigidbody> {
    void operator()(Entity e, Rigidbody &rigidbody)
    {
        ImGui::Checkbox("isStatic", &rigidbody.isStatic);
        ImGui::Text("time rested: %.2f", rigidbody.time_resting);
        ImGui::Checkbox("lock rotation", &rigidbody.isRotationLocked);
        ImGui::Checkbox("use auto mass", &rigidbody.useAutomaticMass);
        ImGui::Indent();
        if(!rigidbody.useAutomaticMass && !rigidbody.isStatic) {
            ImGui::DragFloat("mass", &rigidbody.real_mass, 1.f, 1.f);
            ImGui::DragFloat("inertia", &rigidbody.real_inertia, 1.f, 1.f);
        } else {
            ImGui::Text("%f mass", rigidbody.mass());
            ImGui::Text("%f inertia", rigidbody.inertia());
        }
        ImGui::Unindent();
        ImGui::DragFloat2("velocity", helper::VecToPtr(rigidbody.velocity));
        ImGui::DragFloat("angular_vel", &rigidbody.angular_velocity, M_PI / 50.f);
    }
};
template <> struct Inspector::Inspect<AnimatedSprite> {
    void operator()(Entity e, AnimatedSprite &sprite)
    {
        ImGui::Checkbox("flip horizontal", &sprite.flipX);
        ImGui::Checkbox("flip vertical", &sprite.flipY);
        ImGui::ColorEdit4("sprite tint", helper::VecToPtr(sprite.color));
        if(sprite.color_override.has_value()) {
            ImGui::Indent();
            ImGui::ColorEdit4("sprite color override", helper::VecToPtr(sprite.color_override.value()));
            if(ImGui::Button("no color override")) {
                sprite.color_override = {};
            }
            ImGui::Unindent();
        } else {
            if(ImGui::Button("add color override")) {
                sprite.color_override = glm::vec4();
            }
        }
        ImGui::SliderFloat("speed", &sprite.animation_speed, 0.f, 10.f);
        ImGui::DragFloat2("offset", helper::VecToPtr(sprite.position_offset));
    }
};
template <> struct Inspector::Inspect<Sprite> {
    void operator()(Entity e, Sprite &sprite)
    {
        ImGui::Checkbox("flip horizontal", &sprite.flipX);
        ImGui::Checkbox("flip vertical", &sprite.flipY);
        ImGui::ColorEdit4("sprite tint", helper::VecToPtr(sprite.color));
        if(sprite.color_override.has_value()) {
            ImGui::Indent();
            ImGui::ColorEdit4("sprite color override", helper::VecToPtr(sprite.color_override.value()));
            if(ImGui::Button("no color override")) {
                sprite.color_override = {};
            }
            ImGui::Unindent();
        } else {
            if(ImGui::Button("add color override")) {
                sprite.color_override = glm::vec4();
            }
        }
        ImGui::Text("texture: %s", sprite.textureID().c_str());
    }
};
};
#endif  //  EMP_INSPECTOR_HPP
