#include "sprite.hpp"
#include "debug/log.hpp"
namespace emp {
std::unique_ptr<Buffer> Sprite::s_vertex_buffer;
AABB Sprite::shader_rect() const
{
    AABB actual_rect = rect();
    vec2f texture_size = texture().getSize();
    vec2f standardized_min(actual_rect.min.x / texture_size.x, actual_rect.min.y / texture_size.y);
    vec2f standardized_max(actual_rect.max.x / texture_size.x, actual_rect.max.y / texture_size.y);
    return AABB::CreateMinMax(standardized_min, standardized_max);
}
AABB Sprite::rect() const
{
    if(hframes == 0 || vframes == 0) {
        return m_rect;
    }
    float frame_width = texture().getSize().x / hframes;
    float frame_height = texture().getSize().y / vframes;
    int idx_x = frame % hframes;
    int idx_y = frame / hframes;
    assert(idx_x < hframes && idx_y < vframes && "frame out of spritesheet range");
    return AABB::CreateMinMax({ idx_x * frame_width, idx_y * frame_height },
                              { (idx_x + 1) * frame_width, (idx_y + 1) * frame_height });
}
void Sprite::init(Device &device)
{
    assert(s_vertex_count >= 3 && "Vertex count must be at least 3");
    VkDeviceSize bufferSize = sizeof(s_verticies[0]) * s_vertex_count;
    uint32_t vertexSize = sizeof(s_verticies[0]);

    Buffer stagingBuffer {
        device,
        vertexSize,
        s_vertex_count,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
    };

    stagingBuffer.map();
    stagingBuffer.writeToBuffer((void *)s_verticies);

    s_vertex_buffer = std::make_unique<Buffer>(device, vertexSize, s_vertex_count,
                                               VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    device.copyBuffer(stagingBuffer.getBuffer(), s_vertex_buffer->getBuffer(), bufferSize);
}
void Sprite::bind(VkCommandBuffer commandBuffer)
{
    VkBuffer buffers[] = { s_vertex_buffer->getBuffer() };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
}
void Sprite::draw(VkCommandBuffer commandBuffer)
{
    vkCmdDraw(commandBuffer, s_vertex_count, 1, 0, 0);
}

Sprite::Sprite(Texture tex, vec2f size)
    : m_texture(tex)
    , m_size(size)
{
    auto tex_size = tex.texture().getSize();
    if(size.x == 0 && size.y == 0) {
        m_size = tex_size;
    }
}
};  //  namespace emp
