#include "renderer/Mesh.h"
#include "core/Logger.h"

namespace Engine {

void Mesh::Create(VmaAllocator allocator, VkDevice device, VkCommandPool cmdPool, VkQueue queue,
                  const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
{
    m_IndexCount = static_cast<uint32_t>(indices.size());

    // ── Vertex buffer (staging → GPU) ─────────────────────────
    VkDeviceSize vbSize = sizeof(Vertex) * vertices.size();
    Buffer staging;
    staging.Create(allocator, vbSize, BufferUsage::Staging);
    staging.Upload(vertices.data(), vbSize);

    m_VertexBuffer.Create(allocator, vbSize, BufferUsage::Vertex);
    Buffer::CopyBuffer(device, cmdPool, queue,
                       staging.GetHandle(), m_VertexBuffer.GetHandle(), vbSize);
    staging.Destroy();

    // ── Index buffer (staging → GPU) ──────────────────────────
    VkDeviceSize ibSize = sizeof(uint32_t) * indices.size();
    Buffer ibStaging;
    ibStaging.Create(allocator, ibSize, BufferUsage::Staging);
    ibStaging.Upload(indices.data(), ibSize);

    m_IndexBuffer.Create(allocator, ibSize, BufferUsage::Index);
    Buffer::CopyBuffer(device, cmdPool, queue,
                       ibStaging.GetHandle(), m_IndexBuffer.GetHandle(), ibSize);
    ibStaging.Destroy();

    LOG_INFO("Mesh", "Created mesh: {} vertices, {} indices",
             vertices.size(), indices.size());
}

void Mesh::Destroy() {
    m_VertexBuffer.Destroy();
    m_IndexBuffer.Destroy();
    m_IndexCount = 0;
}

void Mesh::Bind(VkCommandBuffer cmd) const {
    VkBuffer buffers[] = { m_VertexBuffer.GetHandle() };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(cmd, 0, 1, buffers, offsets);
    vkCmdBindIndexBuffer(cmd, m_IndexBuffer.GetHandle(), 0, VK_INDEX_TYPE_UINT32);
}

void Mesh::Draw(VkCommandBuffer cmd) const {
    vkCmdDrawIndexed(cmd, m_IndexCount, 1, 0, 0, 0);
}

Mesh Mesh::CreateCube(VmaAllocator allocator, VkDevice device, VkCommandPool cmdPool, VkQueue queue) {
    // Each face has its own vertices for correct normals
    const glm::vec3 white  = {1.0f, 1.0f, 1.0f};
    const glm::vec3 red    = {1.0f, 0.3f, 0.3f};
    const glm::vec3 green  = {0.3f, 1.0f, 0.3f};
    const glm::vec3 blue   = {0.3f, 0.3f, 1.0f};
    const glm::vec3 yellow = {1.0f, 1.0f, 0.3f};
    const glm::vec3 cyan   = {0.3f, 1.0f, 1.0f};

    std::vector<Vertex> vertices = {
        // Front face (z = +0.5) — RED
        {{-0.5f, -0.5f,  0.5f}, red,    { 0, 0, 1}, {0, 0}},
        {{ 0.5f, -0.5f,  0.5f}, red,    { 0, 0, 1}, {1, 0}},
        {{ 0.5f,  0.5f,  0.5f}, red,    { 0, 0, 1}, {1, 1}},
        {{-0.5f,  0.5f,  0.5f}, red,    { 0, 0, 1}, {0, 1}},

        // Back face (z = -0.5) — GREEN
        {{ 0.5f, -0.5f, -0.5f}, green,  { 0, 0,-1}, {0, 0}},
        {{-0.5f, -0.5f, -0.5f}, green,  { 0, 0,-1}, {1, 0}},
        {{-0.5f,  0.5f, -0.5f}, green,  { 0, 0,-1}, {1, 1}},
        {{ 0.5f,  0.5f, -0.5f}, green,  { 0, 0,-1}, {0, 1}},

        // Top face (y = +0.5) — BLUE
        {{-0.5f,  0.5f,  0.5f}, blue,   { 0, 1, 0}, {0, 0}},
        {{ 0.5f,  0.5f,  0.5f}, blue,   { 0, 1, 0}, {1, 0}},
        {{ 0.5f,  0.5f, -0.5f}, blue,   { 0, 1, 0}, {1, 1}},
        {{-0.5f,  0.5f, -0.5f}, blue,   { 0, 1, 0}, {0, 1}},

        // Bottom face (y = -0.5) — YELLOW
        {{-0.5f, -0.5f, -0.5f}, yellow, { 0,-1, 0}, {0, 0}},
        {{ 0.5f, -0.5f, -0.5f}, yellow, { 0,-1, 0}, {1, 0}},
        {{ 0.5f, -0.5f,  0.5f}, yellow, { 0,-1, 0}, {1, 1}},
        {{-0.5f, -0.5f,  0.5f}, yellow, { 0,-1, 0}, {0, 1}},

        // Right face (x = +0.5) — CYAN
        {{ 0.5f, -0.5f,  0.5f}, cyan,   { 1, 0, 0}, {0, 0}},
        {{ 0.5f, -0.5f, -0.5f}, cyan,   { 1, 0, 0}, {1, 0}},
        {{ 0.5f,  0.5f, -0.5f}, cyan,   { 1, 0, 0}, {1, 1}},
        {{ 0.5f,  0.5f,  0.5f}, cyan,   { 1, 0, 0}, {0, 1}},

        // Left face (x = -0.5) — WHITE
        {{-0.5f, -0.5f, -0.5f}, white,  {-1, 0, 0}, {0, 0}},
        {{-0.5f, -0.5f,  0.5f}, white,  {-1, 0, 0}, {1, 0}},
        {{-0.5f,  0.5f,  0.5f}, white,  {-1, 0, 0}, {1, 1}},
        {{-0.5f,  0.5f, -0.5f}, white,  {-1, 0, 0}, {0, 1}},
    };

    std::vector<uint32_t> indices;
    indices.reserve(36);
    for (uint32_t face = 0; face < 6; face++) {
        uint32_t base = face * 4;
        indices.push_back(base + 0);
        indices.push_back(base + 1);
        indices.push_back(base + 2);
        indices.push_back(base + 2);
        indices.push_back(base + 3);
        indices.push_back(base + 0);
    }

    Mesh mesh;
    mesh.Create(allocator, device, cmdPool, queue, vertices, indices);
    return mesh;
}

} // namespace Engine
