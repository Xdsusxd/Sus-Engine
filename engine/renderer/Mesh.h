#pragma once

#include "renderer/Buffer.h"
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <glm/glm.hpp>
#include <vector>
#include <array>

namespace Engine {

struct Vertex {
    glm::vec3 position;
    glm::vec3 color;
    glm::vec3 normal;
    glm::vec2 uv;

    static VkVertexInputBindingDescription GetBindingDescription() {
        VkVertexInputBindingDescription binding{};
        binding.binding   = 0;
        binding.stride    = sizeof(Vertex);
        binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return binding;
    }

    static std::array<VkVertexInputAttributeDescription, 4> GetAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 4> attrs{};

        // Position
        attrs[0].binding  = 0;
        attrs[0].location = 0;
        attrs[0].format   = VK_FORMAT_R32G32B32_SFLOAT;
        attrs[0].offset   = offsetof(Vertex, position);

        // Color
        attrs[1].binding  = 0;
        attrs[1].location = 1;
        attrs[1].format   = VK_FORMAT_R32G32B32_SFLOAT;
        attrs[1].offset   = offsetof(Vertex, color);

        // Normal
        attrs[2].binding  = 0;
        attrs[2].location = 2;
        attrs[2].format   = VK_FORMAT_R32G32B32_SFLOAT;
        attrs[2].offset   = offsetof(Vertex, normal);

        // UV
        attrs[3].binding  = 0;
        attrs[3].location = 3;
        attrs[3].format   = VK_FORMAT_R32G32_SFLOAT;
        attrs[3].offset   = offsetof(Vertex, uv);

        return attrs;
    }
};

class Mesh {
public:
    void Create(VmaAllocator allocator, VkDevice device, VkCommandPool cmdPool, VkQueue queue,
                const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
    void Destroy();

    void Bind(VkCommandBuffer cmd) const;
    void Draw(VkCommandBuffer cmd) const;

    uint32_t GetIndexCount() const { return m_IndexCount; }

    // ── Primitives ────────────────────────────────────────────
    static Mesh CreateCube(VmaAllocator allocator, VkDevice device, VkCommandPool cmdPool, VkQueue queue);

private:
    Buffer   m_VertexBuffer;
    Buffer   m_IndexBuffer;
    uint32_t m_IndexCount = 0;
};

} // namespace Engine
