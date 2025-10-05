#ifndef EMP_SPATIAL_VISUALIZER_HPP
#define EMP_SPATIAL_VISUALIZER_HPP

#include <functional>
#include <string>
#include "core/entity.hpp"
namespace emp {
class Coordinator;
class Camera;
class SpatialVisualizer {
public:
    bool isOpen = true;
    void draw(const char *title, Coordinator &ECS, std::function<std::string(Entity)> namingFunc, Camera &camera);
};
}

#endif
