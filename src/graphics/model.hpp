#ifndef EMP_MODEL_HPP
#define EMP_MODEL_HPP

#include <optional>
#include <unordered_map>
#include "graphics/texture.hpp"
#include "vulkan/buffer.hpp"
#include "vulkan/device.hpp"

//  libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/glm.hpp>

//  std
#include <memory>
#include <vector>
#include "vertex.hpp"

namespace emp {
class ModelAsset {
public:
    struct Builder {
        std::vector<Vertex> vertices {};
        std::vector<uint32_t> indices {};

        Builder &loadModel(const std::string &filepath);
    };

    ModelAsset(Device &device, const ModelAsset::Builder &builder);
    ~ModelAsset();
    ModelAsset(const ModelAsset &) = delete;
    ModelAsset &operator=(const ModelAsset &) = delete;

    static std::unique_ptr<ModelAsset> createModelFromFile(Device &device, const std::string &filepath);
    void bind(VkCommandBuffer commandBuffer) const;
    void draw(VkCommandBuffer commandBuffer) const;

private:
    Device &device;
    void createVertexBuffers(const std::vector<Vertex> &vertices, Device &device);
    void createIndexBuffers(const std::vector<uint32_t> &indices, Device &device);

    std::unique_ptr<Buffer> vertexBuffer;
    uint32_t vertexCount {};

    bool hasIndexBuffer = false;
    std::unique_ptr<Buffer> indexBuffer;
    uint32_t indexCount {};
};
class Model {
private:
    std::string m_id;
    static std::unordered_map<std::string, std::unique_ptr<ModelAsset>> m_model_table;

public:
    Texture texture;
    std::optional<glm::vec4> color;
    static void destroyAll() { m_model_table.clear(); }
    std::string getID() const { return m_id; }
    ModelAsset &model() { return *m_model_table.at(m_id); }
    static void create(std::string id, Device &device, const ModelAsset::Builder &builder)
    {
        auto model = std::make_unique<ModelAsset>(device, builder);
        assert(!m_model_table.contains(id) && "trying to override existing model id");
        m_model_table[id] = std::move(model);
    }
    static bool isLoaded(std::string id) { return m_model_table.contains(id); }
    Model() { }
    Model(std::string model_id)
        : m_id(model_id)
    {
        assert(m_model_table.contains(m_id) && "model must be first created");
    }
};
}  //  namespace emp

#endif
