#ifndef EMP_VERTEX_HPP
#define EMP_VERTEX_HPP

#include "graphics/utils.hpp"
#include "vulkan/buffer.hpp"
#include "vulkan/device.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

//  std
#include <memory>
#include <vector>
namespace emp {
struct Vertex {
    glm::vec3 position {};
    glm::vec3 color {};
    glm::vec3 normal {};
    glm::vec2 uv {};

    static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
    static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

    bool operator==(const Vertex &other) const
    {
        return position == other.position && color == other.color && normal == other.normal && uv == other.uv;
    }
};
};  //  namespace emp
namespace std {
template <> struct hash<emp::Vertex> {
    size_t operator()(const emp::Vertex &vertex) const
    {
        size_t seed = 0;
        emp::hashCombine(seed, vertex.position, vertex.color, vertex.normal, vertex.uv);
        return seed;
    }
};
}  //  namespace std

#endif
