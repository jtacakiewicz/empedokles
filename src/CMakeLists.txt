set(SOURCE_FILES
    #main.cpp
    debug/log.cpp

    compute/compute_manager.cpp

    math/types.cpp
    math/math_func.cpp
    math/geometry_func.cpp

    memory/linear_allocator.cpp
    memory/pool_allocator.cpp

    scene/app.cpp
    scene/register_scene_types.cpp
    scene/transform.cpp


    core/entity_manager.cpp
    core/coordinator.cpp
    core/layer.cpp

    physics/constraint.cpp
    physics/collider.cpp
    physics/rigidbody.cpp
    physics/physics_system.cpp

    graphics/imgui/imgui_emp_impl.cpp
    # graphics/systems/point_light_system.cpp
    graphics/render_systems/simple_render_system.cpp
    graphics/render_systems/particle_render_system.cpp
    gui/console.cpp
    gui/overlay.cpp
    gui/log_window.cpp
    gui/spatial_visualizer.cpp
    gui/gui_manager.cpp
    gui/editor/tree-view.cpp

    vulkan/buffer.cpp
    vulkan/descriptors.cpp
    vulkan/device.cpp
    vulkan/pipeline.cpp
    vulkan/swap_chain.cpp

    graphics/camera.cpp
    graphics/model_system.cpp
    graphics/vertex.cpp
    graphics/model.cpp
    graphics/renderer.cpp
    graphics/texture.cpp
    graphics/sprite.cpp
    graphics/animated_sprite.cpp
    graphics/animated_sprite_system.cpp
    graphics/sprite_system.cpp
    graphics/particle_system.cpp

    audio/sound_asset.cpp
    audio/sound_source.cpp

    io/keyboard_controller.cpp
    io/window.cpp
)
set(HEADER_FILES
    memory/parallel_iterator.hpp
    memory/pool_allocator.hpp
    memory/allocator.hpp
    memory/relative_pointer.hpp

    debug/log.hpp
    debug/debug.hpp

    compute/multithreading/thread_pool.hpp
    compute/compute_manager.hpp


    math/types.hpp
    math/shapes/AABB.hpp
    math/shapes/circle.hpp
    math/shapes/concave_polygon.hpp
    math/shapes/convex_polygon.hpp
    math/shapes/ray.hpp
    math/math_func.hpp
    math/math_defs.hpp
    math/math.hpp
    math/geometry_func.hpp

    physics/constraint.hpp
    physics/collider.hpp
    physics/rigidbody.hpp
    physics/material.hpp
    physics/physics_system.hpp

    scene/app.hpp
    scene/register_scene_types.hpp
    scene/hierarchy.hpp
    scene/scene_defs.hpp
    scene/transform.hpp

    scene/behaviour.hpp

    core/entity.hpp
    core/entity_manager.hpp
    core/component.hpp
    core/component_array.hpp
    core/component_manager.hpp
    core/system_manager.hpp
    core/system_base.hpp
    core/system.hpp
    core/coordinator.hpp
    core/layer.hpp

    templates/observer.hpp
    templates/set.hpp
    templates/stack_linked_list.hpp
    templates/disjoint_set.hpp
    templates/free_list.hpp
    templates/finite_state_machine.hpp
    templates/relative_vector.hpp
    templates/relative_string.hpp
    templates/quad_tree.hpp
    templates/sweep_line.hpp

    debug/log.hpp
    debug/debug.hpp

    graphics/imgui/imgui_emp_impl.hpp
    # graphics/systems/point_light_system.hpp
    graphics/render_systems/simple_render_system.hpp
    graphics/render_systems/particle_render_system.hpp
    gui/console.hpp
    gui/overlay.hpp
    gui/log_window.hpp
    gui/spatial_visualizer.hpp
    gui/editor/inspector.hpp
    gui/editor/tree-view.hpp
    gui/gui_manager.hpp

    vulkan/buffer.hpp
    vulkan/descriptors.hpp
    vulkan/device.hpp
    vulkan/pipeline.hpp
    vulkan/swap_chain.hpp

    graphics/camera.hpp
    graphics/frame_info.hpp
    graphics/model_system.hpp
    graphics/vertex.hpp
    graphics/model.hpp
    graphics/renderer.hpp
    graphics/texture.hpp
    graphics/sprite.hpp
    graphics/animated_sprite.hpp
    graphics/animated_sprite_system.hpp
    graphics/sprite_system.hpp
    graphics/utils.hpp
    graphics/particle_system.hpp
    graphics/particle_emit_queue.hpp

    audio/sound_asset.hpp
    audio/sound_source.hpp

    io/keyboard_controller.hpp
    io/window.hpp

    external/tinyobjloader/tiny_obj_loader.h
    external/stbimage/stb_image.h
)

include_directories(.)
include_directories(external/tinyobjloader)
include_directories(external/stbimage)
include_directories(external/dr_libs)
add_library(empedokles SHARED
    ${SOURCE_FILES} ${HEADER_FILES}
)
target_include_directories(empedokles PUBLIC external/dr_libs)
target_compile_options(empedokles PUBLIC -g)

