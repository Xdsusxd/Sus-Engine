#pragma once

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <cstdint>
#include <cstring>

namespace Engine {

enum class BufferUsage {
    Vertex,
    DynamicVertex,
    Index,
    Uniform,
    Staging
};

class Buffer {
public:
    void Create(VmaAllocator allocator, VkDeviceSize size, BufferUsage usage);
    void Destroy();

    void Upload(const void* data, VkDeviceSize size);

    // Copy data from staging buffer to this buffer via command buffer
    static void CopyBuffer(VkDevice device, VkCommandPool commandPool, VkQueue queue,
                           VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

    VkBuffer      GetHandle() const { return m_Buffer; }
    VkDeviceSize  GetSize()   const { return m_Size; }

private:
    VmaAllocator  m_Allocator  = VK_NULL_HANDLE;
    VkBuffer      m_Buffer     = VK_NULL_HANDLE;
    VmaAllocation m_Allocation = VK_NULL_HANDLE;
    VkDeviceSize  m_Size       = 0;
};

} // namespace Engine
